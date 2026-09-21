// PrismExport addr=0x18001e000 name=anon ghidra=FUN_18001e000 status=clean lines=116

ulonglong FUN_18001e000(longlong *param_1)

{
  longlong lVar1;
  undefined8 uVar2;
  longlong lVar3;
  longlong lVar4;
  uint uVar5;
  undefined4 uVar6;
  uint uVar7;
  ulonglong in_RAX;
  ulonglong uVar8;
  undefined8 uVar9;
  ulonglong uVar10;
  ulonglong uVar11;
  bool bVar12;
  ulonglong local_res8;
  
  lVar3 = *param_1;
  if (lVar3 != 0) {
    uVar8 = (ulonglong)*(uint *)(lVar3 + 4);
    do {
      uVar5 = (uint)uVar8;
      LOCK();
      uVar7 = *(uint *)(lVar3 + 4);
      if (uVar5 == uVar7) {
        *(uint *)(lVar3 + 4) = uVar5 | 1;
      }
      else {
        uVar8 = (ulonglong)uVar7;
      }
      UNLOCK();
    } while (uVar5 != uVar7);
    if ((uVar8 & 1) == 0) {
      uVar6 = _Thrd_id();
      *(undefined4 *)(lVar3 + 0x18) = uVar6;
LAB_18001e051:
      do {
        uVar8 = *(ulonglong *)(lVar3 + 8);
        do {
          while (uVar11 = uVar8, uVar7 = (uint)uVar11 & 3, (uVar11 & 3) == 0) {
            LOCK();
            uVar8 = *(ulonglong *)(lVar3 + 8);
            bVar12 = uVar11 == uVar8;
            if (bVar12) {
              *(ulonglong *)(lVar3 + 8) = uVar11 | 1;
              uVar8 = uVar11;
            }
            UNLOCK();
            if (bVar12) {
              *(ulonglong *)(lVar3 + 0x10) = uVar11;
              uVar9 = __std_atomic_notify_all_direct();
              if (uVar11 == 0) {
                LOCK();
                uVar2 = *(undefined8 *)(lVar3 + 8);
                *(undefined8 *)(lVar3 + 8) = 0;
                UNLOCK();
                if (((byte)uVar2 & 3) == 2) {
                  uVar9 = __std_atomic_notify_all_direct(lVar3 + 8);
                }
                in_RAX = CONCAT71((int7)((ulonglong)uVar9 >> 8),1);
                goto LAB_18001e13f;
              }
              lVar4 = *(longlong *)(uVar11 + 8);
              *(undefined8 *)(uVar11 + 8) = 0;
              if (lVar4 != 0) {
                *(undefined8 *)(lVar4 + 0x10) = 0;
              }
              LOCK();
              lVar1 = *(longlong *)(lVar3 + 8);
              *(longlong *)(lVar3 + 8) = lVar4;
              UNLOCK();
              if (((byte)lVar1 & 3) == 2) {
                __std_atomic_notify_all_direct(lVar3 + 8);
              }
              (**(code **)(uVar11 + 0x18))(uVar11);
              goto LAB_18001e051;
            }
          }
          if (uVar7 != 1) {
            uVar10 = uVar11;
            if (uVar7 != 2) {
                    /* WARNING: Subroutine does not return */
              abort();
            }
            break;
          }
          uVar10 = uVar11 & 0xfffffffffffffffe | 2;
          LOCK();
          uVar8 = *(ulonglong *)(lVar3 + 8);
          bVar12 = uVar11 == uVar8;
          if (bVar12) {
            *(ulonglong *)(lVar3 + 8) = uVar10;
            uVar8 = uVar11;
          }
          UNLOCK();
        } while (!bVar12);
        local_res8 = uVar10;
        if (uVar10 == *(ulonglong *)(lVar3 + 8)) {
          do {
            __std_atomic_wait_direct(lVar3 + 8,&local_res8,8,0xffffffff);
          } while (local_res8 == *(ulonglong *)(lVar3 + 8));
        }
      } while( true );
    }
    in_RAX = uVar8 & 0xffffffffffffff00;
LAB_18001e13f:
    if ((char)in_RAX != '\0') {
      return CONCAT71((int7)(in_RAX >> 8),1);
    }
  }
  return in_RAX & 0xffffffffffffff00;
}

