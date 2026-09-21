// PrismExport addr=0x18004da00 name=guess ghidra=FUN_18004da00 status=clean lines=32

void FUN_18004da00(undefined4 param_1)

{
  undefined8 *puVar1;
  undefined8 *puVar2;
  int iVar3;
  undefined *puVar4;
  
  puVar4 = FUN_18004d0f0();
  iVar3 = _Mtx_lock(puVar4);
  if (iVar3 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (*(int *)(puVar4 + 0x4c) != 0x7fffffff) {
    puVar1 = *(undefined8 **)(puVar4 + 0xf8);
    for (puVar2 = (undefined8 *)*puVar1; puVar2 != puVar1; puVar2 = (undefined8 *)*puVar2) {
      LOCK();
      *(undefined4 *)(puVar2[6] + 0x40) = param_1;
      UNLOCK();
    }
    *(undefined4 *)(puVar4 + 0x178) = param_1;
    _Mtx_unlock(puVar4);
    return;
  }
  *(undefined4 *)(puVar4 + 0x4c) = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
  std::_Throw_Cpp_error(6);
}

