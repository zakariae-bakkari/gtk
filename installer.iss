; Inno Setup Script for Deep Shark Attack
; This script bundles the content of the 'release' folder created by deploy.ps1

[Setup]
AppId={{D33P-5H4RK-4774-CK-BTK-001}}
AppName=Deep Shark Attack
AppVersion=1.0
AppPublisher=zakariae''s groupe
DefaultDirName={autopf}\DeepSharkAttack
DefaultGroupName=Deep Shark Attack
AllowNoIcons=yes
; Specify the path to your app icon
SetupIconFile={#SourcePath}\resources\icons\app.ico
Compression=lzma
SolidCompression=yes
WizardStyle=modern
OutputDir={#SourcePath}\installer_output
OutputBaseFilename=DeepSharkAttack_Setup

[Languages]
Name: "english"; MessagesFile: "compiler:Default.isl"
Name: "french"; MessagesFile: "compiler:Languages\French.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
; Copy everything from the release folder
Source: "{#SourcePath}\release\DeepSharkAttack.exe"; DestDir: "{app}"; Flags: ignoreversion
Source: "{#SourcePath}\release\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs
; NOTE: Don't use "Flags: ignoreversion" on shared system files

[Icons]
Name: "{group}\Deep Shark Attack"; Filename: "{app}\DeepSharkAttack.exe"
Name: "{autodesktop}\Deep Shark Attack"; Filename: "{app}\DeepSharkAttack.exe"; Tasks: desktopicon

[Run]
Filename: "{app}\DeepSharkAttack.exe"; Description: "{cm:LaunchProgram,Deep Shark Attack}"; Flags: nowait postinstall
