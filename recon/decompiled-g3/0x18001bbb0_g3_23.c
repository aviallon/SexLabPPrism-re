// PrismExport addr=0x18001bbb0 name=g3_23 ghidra=FUN_18001bbb0 status=clean lines=26

undefined8 *
FUN_18001bbb0(undefined8 param_1,undefined8 param_2,undefined8 param_3,undefined8 param_4)

{
  int iVar1;
  
  if (DAT_18009c061 == '\0') {
    iVar1 = _Mtx_lock(&DAT_180095050);
    if (iVar1 != 0) {
                    /* WARNING: Subroutine does not return */
      std::_Throw_Cpp_error(5);
    }
    if (DAT_18009509c == 0x7fffffff) {
      DAT_18009509c = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
      std::_Throw_Cpp_error(6);
    }
    FUN_18001c970(&DAT_18009c270,param_2,param_3,param_4);
    DAT_18009c061 = '\x01';
    _Mtx_unlock(&DAT_180095050);
  }
  return &DAT_18009c270;
}

