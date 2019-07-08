This UEFI tool for ITE EC RAM View

1. Add Application item to AppPkg.dsc for compile .c file
//---------------------------------------------------------------------------------------------
[Components]

#### Sample Applications.
  AppPkg/Applications/Hello/Hello.inf        # No LibC includes or functions.
  AppPkg/Applications/Main/Main.inf          # Simple invocation. No other LibC functions.
  AppPkg/Applications/Enquire/Enquire.inf    #
  AppPkg/Applications/ArithChk/ArithChk.inf  #
  AppPkg/Applications/RamView/RamView.inf    # This is ITE RAM View Tool
//---------------------------------------------------------------------------------------------

2. compile in UDK2017
    a. C:\MyWorkspace>Edk2Setup.bat --nt32 X64
    b. C:\MyWorkspace>Build -a X64 -p AppPkg\AppPkg.dsc -m AppPkg\Applications\RamView\RamView.inf