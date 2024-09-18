# Crédits : Jérôme LÉCUYER - Cohorte-20
# Réalisé pour nushell 0.97.1

# =================================================>
# Update Configs Here

# example : 
# set AIBOOTCAMP2_ENGINE_DIR=D:\AIBootcamp2\AIBootcamp2\Releases
# set AIBOT_PATH=D:\AIBootcamp2\LesOutardes\build-solution-x64\Debug\lesoutardes.dll
# set TEAM=LesOutardes

let $AIBOOTCAMP2_ENGINE_DIR = "../AIBootCamp2-2024.2.01/Release"
let $AIBOT_PATH = "./build-solution-x64/Debug/myfirstaibot.dll"
let $TEAM = "Les Architectes"

# <=================================================

# Build le projet Visual Studio
def Build [] {
    msbuild build-solution-x64\myfirstaibot.vcxproj
}

# Obtenir le nom des maps
def MapNames [] { [
    L_000 L_001 L_002 L_003 L_004 L_005
    L_010 L_011 L_012 L_013
    L_020 L_021 L_022 L_023
    L_030 L_031 L_032 L_033 L_034
    L_040 L_041 L_042
    L_050
] }

# Exécuter la simulation sur une map
def Play [
    mapname: string@MapNames # Name of the map to play like L_xxx
    ConnectToProcessDelay: number = -1 # Time to wait for connection with Visual Studio, in s
    InitTime: number = -1 # in ms
    TurnTime: number = -1 # Maximum time of a turn before timeout, in ms
] {
    print $"Exécution de ($mapname)"
    python PlayLocalMatch.py $mapname $AIBOOTCAMP2_ENGINE_DIR $AIBOT_PATH $TEAM $ConnectToProcessDelay $InitTime $TurnTime
}

# Exécuter sur toutes les map, puis donner un compte rendu
def PlayAll [] {
    let results = (MapNames | enumerate
    | rename index mapname
    | insert result {
        try {
            Play $in.mapname
        } catch {
            return {resultData: {hasWin: false, resultPayload: 'mapname invalide'}}
        }
        let $lastReplay = (LastReplay)
        if $lastReplay == null { return {resultData: {hasWin: false, resultPayload: 'Exception'}} }
        return $lastReplay.content
    }
    | insert hasWin { $in.result.resultData.hasWin }
    | insert resultPayload { $in.result.resultData.resultPayload }
    | reject result
    )
    return $results
}

# Lancer en mode Debug (attendre 8 secondes)
def Debug [mapname: string@MapNames] {
    Play $mapname 8 -1 600_000
}

# Obtenir les informations du dernier replay
def LastReplay [] {
    let $replayfolder: string = (ls Replays | where type == dir | sort-by modified | last | get name)
    let $replayfile: string = (ls $replayfolder | where name =~ .replay | sort-by modified | last | get name)
    try {
        let content: table = (open $replayfile | from json)
        return {replayfile:$replayfile content:$content}
    } catch {
        print "Le fichier de replay est invalide. Le programme a planté ?"
        return null
    }
}

# Ouvrir la visu avec le dernier replay
def OpenLastReplay [] {
    let $lastReplay = (LastReplay)
    if $lastReplay != null {
        print $"Ouverture de ($lastReplay.replayfile)"
        start $lastReplay.replayfile
    }
}
alias ReplayLast = OpenLastReplay

# Compiler, lancer la simulation et ouvrir le résultat
def PlayAndReplay [mapname: string@MapNames] {
    Build
    Play $mapname
    print (LastReplay)
    OpenLastReplay
}

# Reconstruire le projet pour Visual Studio en x64
def RebuildSLN [] {
    start rebuild_vis_sln_x64.bat
}

# Ouvrir le projet avec Visual Studio
def OpenSLN [] {
    start ./build-solution-x64/MyFirstAIBot.sln
}

# Générer l'archive
def GenSubmit [] {
    python GenSubmit.py $TEAM
}