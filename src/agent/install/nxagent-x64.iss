; Script generated by the Inno Setup Script Wizard.
; SEE THE DOCUMENTATION FOR DETAILS ON CREATING INNO SETUP SCRIPT FILES!

[Setup]
#include "setup.iss"
OutputBaseFilename=nxagent-0.2.24-x64
ArchitecturesInstallIn64BitMode=x64
ArchitecturesAllowed=x64

[Files]
Source: "..\..\libnetxms\Release64\libnetxms.dll"; DestDir: "{app}\bin"; BeforeInstall: StopService; Flags: ignoreversion
Source: "..\..\libnxlp\Release64\libnxlp.dll"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\core\Release64\nxagentd.exe"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\winnt\Release64\winnt.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\winperf\Release64\winperf.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\wmi\Release64\wmi.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\ping\Release64\ping.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\portCheck\Release64\portcheck.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\logscan\Release64\logscan.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\logwatch\Release64\logwatch.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\sms\Release64\sms.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\ups\Release64\ups.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\ecs\Release64\ecs.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\subagents\odbcquery\Release64\odbcquery.nsm"; DestDir: "{app}\bin"; Flags: ignoreversion
Source: "..\..\..\contrib\nxagentd.conf-dist"; DestDir: "{app}\etc"; Flags: ignoreversion
Source: "..\..\install\windows-x64\files\libeay32.dll"; DestDir: "{app}\bin"; Flags: ignoreversion

#include "common.iss"

Function GetCustomCmdLine(Param: String): String;
Begin
  Result := '';
End;

