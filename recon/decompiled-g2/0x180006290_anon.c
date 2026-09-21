// PrismExport addr=0x180006290 name=anon ghidra=FUN_180006290 status=clean lines=45

undefined8 FUN_180006290(undefined8 *param_1)

{
  int iVar1;
  int *piVar2;
  longlong lVar3;
  int *local_res8;
  
  piVar2 = (int *)*param_1;
  *param_1 = 0;
  lVar3 = param_1[1];
  local_res8 = piVar2;
  if (piVar2 == (int *)0x0) {
    FUN_180013a20(lVar3,(longlong *)&local_res8);
  }
  else {
    LOCK();
    *piVar2 = *piVar2 + 1;
    UNLOCK();
    FUN_180013a20(lVar3,(longlong *)&local_res8);
    LOCK();
    iVar1 = *piVar2;
    *piVar2 = *piVar2 + -1;
    UNLOCK();
    if (iVar1 == 1) {
      free(piVar2);
    }
  }
  _Cnd_do_broadcast_at_thread_exit();
  piVar2 = (int *)*param_1;
  if (piVar2 != (int *)0x0) {
    LOCK();
    iVar1 = *piVar2;
    *piVar2 = *piVar2 + -1;
    UNLOCK();
    if (iVar1 == 1) {
      free(piVar2);
    }
  }
  free(param_1);
  return 0;
}

