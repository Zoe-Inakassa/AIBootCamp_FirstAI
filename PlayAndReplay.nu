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
let $TEAM = "LesArchitectes"

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
    let now = (date now)
    let replayfile = $"($now | format date %Y%m%d_%H%M%S)"
    print $"    TEAM       = ($TEAM)"
    print $"    replayfile = ($replayfile)"

    let AIBootCampExe = $"($AIBOOTCAMP2_ENGINE_DIR)/AIBootCamp2.exe"
    let options = ([
        $"-dllpath ($AIBOT_PATH)"
        $"-mode match"
        $"-scene ($mapname)"
        $"-team ($TEAM)"
        $"-replayfile ($replayfile)"
        $"-connecttoprocessdelay ($ConnectToProcessDelay)"
        $"-initdelay ($InitTime)"
        $"-turndelay ($TurnTime)"
        "-quit"
        "-batchmode"
    ] | str join ' ')
    
    let $result = (nu -n --no-history -c $"($AIBootCampExe) ($options)" | complete)
    print $result

    if ($result.exit_code == 0) {
        print "Match Completed : Victory!"
    } else {
        let exitMessages = {
            "-2": $"Invalid Map Name ($mapname)"
            "-1": "Match Completed : Failure (-1)"
            "3": "Match Completed : Failure (3)"
        }
        let message = ($exitMessages
            | get $"($result.exit_code)" --ignore-errors
            | default $"Erreur d'exécution: ($result.exit_code)")
        print $message
        }
    return $result.exit_code
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
        if $lastReplay.ok == false { return {resultData: {hasWin: false, resultPayload: 'Exception'}} }
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
    Play $mapname 8 600_000 600_000
}

# Obtenir les informations du dernier replay
def LastReplay [] {
    let $replayfolder: string = (ls Replays | where type == dir and name =~ "2024...._......" | sort-by modified | last | get name)
    let $replayfile: string = (ls $replayfolder | where name =~ .replay | sort-by modified | last | get name)
    let $logfile: string = (ls $replayfolder | where name =~ .log | sort-by modified | last | get name)
    try {
        let content: table = (open $replayfile | from json)
        return {
            replayfile:$replayfile
            content:$content
            logfile:$logfile
            ok:true
            nbTurns: ($content.turnsData | length)
        }
    } catch {
        print "Le fichier de replay est invalide. Le programme a planté ?"
        return {
            replayfile:$replayfile
            content:null
            logfile:$logfile
            ok:false
            nbTurns: NaN
        }
    }
}

def FixReplay [$replayfile: string] {
    $'](char newline)}' | save --append $replayfile
}

# Ouvrir la visu avec le dernier replay
def OpenLastReplay [$tryfix = true] {
    let $lastReplay = (LastReplay)
    if $lastReplay.ok != true {
        if $tryfix {
            print $"Tentative de correction de ($lastReplay.replayfile)"
            FixReplay $lastReplay.replayfile
            return (OpenLastReplay false)
        }
        print $"Le fichier de replay est corrompu ($lastReplay.replayfile)"
    } else {
        print $"Ouverture de ($lastReplay.replayfile)"
        start $lastReplay.replayfile
    }
}
alias ReplayLast = OpenLastReplay

# Compiler, lancer la simulation et ouvrir le résultat
def PlayAndReplay [mapname: string@MapNames] {
    Build
    let $exit_code = (Play $mapname)
    if ($exit_code == -2) { return } # Invalid Map Name
    let $lastReplay = (LastReplay)
    print $lastReplay
    if ($lastReplay.ok == true) {
        OpenLastReplay
    }
}

# Reconstruire le projet pour Visual Studio en x64
def RebuildSLN [] {
    rebuild_vis_sln_x64.bat
}

# Ouvrir le projet avec Visual Studio
def OpenSLN [] {
    start ./build-solution-x64/MyFirstAIBot.sln
}

# Générer l'archive
def GenSubmit [] {
    python GenSubmit.py $TEAM
}