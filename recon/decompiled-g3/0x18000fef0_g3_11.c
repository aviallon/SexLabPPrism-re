// PrismExport addr=0x18000fef0 name=g3_11 ghidra=FUN_18000fef0 status=clean lines=83

undefined4 * FUN_18000fef0(undefined4 *param_1)

{
  int iVar1;
  int *piVar2;
  int *piVar3;
  uintptr_t uVar4;
  int *_Memory;
  int *piVar5;
  
  piVar5 = (int *)0x0;
  *(undefined8 *)(param_1 + 6) = 0;
  *(undefined8 *)(param_1 + 8) = 0;
  *(undefined8 *)(param_1 + 10) = 0;
  *(undefined8 *)(param_1 + 0xc) = 0;
  *(undefined8 *)(param_1 + 0xe) = 0;
  *(undefined8 *)(param_1 + 0x10) = 0;
  *(undefined8 *)(param_1 + 2) = 0;
  *(undefined8 *)(param_1 + 4) = 0;
  param_1[0x12] = 0xffffffff;
  *param_1 = 2;
  param_1[0x13] = 0;
  *(undefined8 *)(param_1 + 0x14) = 0;
  *(undefined8 *)(param_1 + 0x16) = 0;
  *(undefined8 *)(param_1 + 0x18) = 0;
  *(undefined8 *)(param_1 + 0x1a) = 0;
  *(undefined8 *)(param_1 + 0x1c) = 0;
  *(undefined8 *)(param_1 + 0x1e) = 0;
  *(undefined8 *)(param_1 + 0x20) = 0;
  *(undefined8 *)(param_1 + 0x22) = 0;
  *(undefined8 *)(param_1 + 0x24) = 0;
  *(undefined8 *)(param_1 + 0x26) = 0;
  *(undefined8 *)(param_1 + 0x28) = 0;
  *(undefined8 *)(param_1 + 0x2a) = 0;
  *(uintptr_t *)(param_1 + 0x2c) = 0;
  *(undefined8 *)(param_1 + 0x2e) = 0;
  piVar2 = (int *)FUN_180050418(0x20);
  _Memory = piVar5;
  if (piVar2 != (int *)0x0) {
    *piVar2 = 1;
    piVar2[1] = 2;
    piVar2[2] = 0;
    piVar2[3] = 0;
    piVar2[4] = 0;
    piVar2[5] = 0;
    piVar2[6] = 0;
    _Memory = piVar2;
  }
  *(int **)(param_1 + 0x30) = _Memory;
  if (_Memory != (int *)0x0) {
    LOCK();
    *_Memory = *_Memory + 1;
    UNLOCK();
  }
  piVar3 = (int *)FUN_180050418(0x10);
  piVar2 = piVar5;
  if (piVar3 != (int *)0x0) {
    *(int **)piVar3 = _Memory;
    *(undefined4 **)(piVar3 + 2) = param_1;
    piVar2 = piVar3;
    _Memory = piVar5;
  }
  uVar4 = _beginthreadex((void *)0x0,0,FUN_180006290,piVar2,0,param_1 + 0x2e);
  *(uintptr_t *)(param_1 + 0x2c) = uVar4;
  if (uVar4 == 0) {
    param_1[0x2e] = 0;
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(6);
  }
  if (_Memory != (int *)0x0) {
    LOCK();
    iVar1 = *_Memory;
    *_Memory = *_Memory + -1;
    UNLOCK();
    if (iVar1 == 1) {
      free(_Memory);
    }
  }
  return param_1;
}

