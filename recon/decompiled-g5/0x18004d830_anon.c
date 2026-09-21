// PrismExport addr=0x18004d830 name=anon ghidra=FUN_18004d830 status=clean lines=82

void FUN_18004d830(longlong param_1,longlong *param_2,undefined8 param_3,undefined8 param_4)

{
  longlong lVar1;
  undefined8 *puVar2;
  undefined8 *puVar3;
  longlong *plVar4;
  longlong *plVar5;
  code *pcVar6;
  int iVar7;
  longlong *plVar8;
  undefined8 uVar9;
  undefined8 *puVar10;
  undefined8 *puVar11;
  longlong *local_res8;
  longlong *local_res10;
  undefined1 local_res18 [8];
  undefined1 local_res20 [8];
  
  local_res10 = param_2;
  iVar7 = _Mtx_lock();
  if (iVar7 != 0) {
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(5);
  }
  if (*(int *)(param_1 + 0x4c) == 0x7fffffff) {
    *(undefined4 *)(param_1 + 0x4c) = 0x7ffffffe;
                    /* WARNING: Subroutine does not return */
    std::_Throw_Cpp_error(6);
  }
  lVar1 = *param_2;
  *param_2 = 0;
  puVar2 = *(undefined8 **)(param_1 + 0x170);
  *(longlong *)(param_1 + 0x170) = lVar1;
  if (puVar2 != (undefined8 *)0x0) {
    (**(code **)*puVar2)(puVar2,1);
  }
  puVar2 = *(undefined8 **)(param_1 + 0xf8);
  puVar3 = (undefined8 *)*puVar2;
  do {
    if (puVar3 == puVar2) {
      _Mtx_unlock(param_1);
      puVar2 = (undefined8 *)*param_2;
      if (puVar2 != (undefined8 *)0x0) {
        (**(code **)*puVar2)(puVar2,1);
      }
      return;
    }
    lVar1 = puVar3[6];
    plVar8 = (longlong *)
             (**(code **)(**(longlong **)(param_1 + 0x170) + 0x10))
                       (*(longlong **)(param_1 + 0x170),local_res18);
    puVar11 = *(undefined8 **)(lVar1 + 0x28);
    puVar10 = *(undefined8 **)(lVar1 + 0x30);
    if (puVar11 != puVar10) {
      do {
        plVar4 = (longlong *)*puVar11;
        puVar11 = puVar11 + 2;
        plVar5 = (longlong *)*plVar8;
        pcVar6 = *(code **)(*plVar4 + 0x20);
        if (puVar11 == puVar10) {
          *plVar8 = 0;
          local_res8 = plVar5;
          (*pcVar6)(plVar4,&local_res8);
          break;
        }
        uVar9 = (**(code **)(*plVar5 + 0x10))(plVar5,local_res20);
        (*pcVar6)(plVar4,uVar9);
        puVar10 = *(undefined8 **)(lVar1 + 0x30);
      } while (puVar11 != puVar10);
    }
    puVar11 = (undefined8 *)*plVar8;
    if (puVar11 != (undefined8 *)0x0) {
      (**(code **)*puVar11)(puVar11,1);
    }
    puVar3 = (undefined8 *)*puVar3;
    param_2 = local_res10;
  } while( true );
}

