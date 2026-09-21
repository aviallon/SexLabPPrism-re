// PrismExport addr=0x180010b20 name=anon ghidra=FUN_180010b20 status=clean lines=63

void FUN_180010b20(undefined8 *param_1)

{
  uint *puVar1;
  uint uVar2;
  int *_Memory;
  int iVar3;
  int iVar4;
  undefined4 local_18;
  undefined4 uStack_14;
  undefined4 uStack_10;
  undefined4 uStack_c;
  
  if (*(int *)(param_1 + 1) != 0) {
    FUN_18001e000(param_1 + 2);
    if (*(int *)(param_1 + 1) == 0) {
                    /* WARNING: Subroutine does not return */
      std::_Throw_Cpp_error(1);
    }
    iVar4 = *(int *)(param_1 + 1);
    iVar3 = _Thrd_id();
    if (iVar4 == iVar3) {
                    /* WARNING: Subroutine does not return */
      std::_Throw_Cpp_error(5);
    }
    local_18 = *(undefined4 *)param_1;
    uStack_14 = *(undefined4 *)((longlong)param_1 + 4);
    uStack_10 = *(undefined4 *)(param_1 + 1);
    uStack_c = *(undefined4 *)((longlong)param_1 + 0xc);
    iVar4 = _Thrd_join(&local_18,0);
    if (iVar4 != 0) {
                    /* WARNING: Subroutine does not return */
      std::_Throw_Cpp_error(2);
    }
    *param_1 = 0;
    param_1[1] = 0;
  }
  _Memory = (int *)param_1[2];
  if (_Memory != (int *)0x0) {
    LOCK();
    puVar1 = (uint *)(_Memory + 1);
    uVar2 = *puVar1;
    *puVar1 = *puVar1 - 2;
    UNLOCK();
    if ((uVar2 & 0xfffffffe) == 2) {
      LOCK();
      iVar4 = *_Memory;
      *_Memory = *_Memory + -1;
      UNLOCK();
      if (iVar4 == 1) {
        free(_Memory);
      }
    }
  }
  if (*(int *)(param_1 + 1) != 0) {
                    /* WARNING: Subroutine does not return */
    terminate();
  }
  return;
}

