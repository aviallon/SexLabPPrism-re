// PrismExport addr=0x180044740 name=.?AV?$_Fmt_iterator_buffer@V?$back_insert_iterator@V?$basic_string@DU?$char_traits@D@std@@V?$allocator@D@2@@std@@@std@@DU_Fmt_buffer_traits@2@@std@@::vtable[0] ghidra=FUN_180044740 status=clean lines=163

/* WARNING: Function: __security_check_cookie replaced with injection: security_check_cookie */

ulonglong * FUN_180044740(ulonglong *param_1,undefined8 *param_2,ulonglong *param_3)

{
  ulonglong uVar1;
  undefined8 *puVar2;
  ulonglong uVar3;
  code *pcVar4;
  undefined8 uVar5;
  byte bVar6;
  longlong lVar7;
  void *pvVar8;
  ulonglong uVar9;
  void *pvVar10;
  undefined1 *puVar11;
  undefined1 *puVar12;
  void *_Dst;
  void *pvVar13;
  ulonglong uVar14;
  undefined1 auStack_1f8 [8];
  undefined1 auStack_1f0 [24];
  undefined4 local_1d8;
  ulonglong *local_1d0;
  ulonglong local_48;
  
  puVar11 = auStack_1f8;
  local_48 = DAT_180095900 ^ (ulonglong)auStack_1f8;
  _Dst = (void *)0x0;
  *param_1 = 0;
  param_1[1] = 0;
  param_1[2] = 0;
  param_1[3] = 0xf;
  *(undefined1 *)param_1 = 0;
  local_1d8 = 1;
  pvVar13 = (void *)*param_3;
  pvVar8 = _Dst;
  if (pvVar13 != (void *)0x0) {
    pvVar10 = _Dst;
    do {
      uVar3 = *(ulonglong *)(param_3[1] + (longlong)pvVar10 * 8);
      bVar6 = (byte)(uVar3 >> 0x3c);
      if (bVar6 == 0xc) {
        pvVar8 = (void *)((longlong)pvVar8 +
                         *(longlong *)
                          (param_3[1] + (uVar3 & 0xfffffffffffffff) + 8 + (longlong)pvVar13 * 8));
      }
      else if (bVar6 == 0xb) {
        pvVar8 = (void *)((longlong)pvVar8 + 0x20);
      }
      else {
        pvVar8 = (void *)((longlong)pvVar8 + 8);
      }
      pvVar10 = (void *)((longlong)pvVar10 + 1);
    } while (pvVar10 < pvVar13);
  }
  uVar14 = param_2[1] + (longlong)pvVar8;
  uVar3 = param_1[3];
  puVar12 = auStack_1f8;
  local_1d0 = param_1;
  if (uVar14 <= uVar3) goto LAB_18004490c;
  pvVar13 = (void *)0x7fffffffffffffff;
  if (0x7fffffffffffffff < uVar14) {
                    /* WARNING: Subroutine does not return */
    FUN_180019860();
  }
  pvVar8 = (void *)(uVar14 | 0xf);
  if ((pvVar8 < (void *)0x8000000000000000) && (uVar3 <= 0x7fffffffffffffff - (uVar3 >> 1))) {
    pvVar10 = (void *)((uVar3 >> 1) + uVar3);
    pvVar13 = pvVar8;
    if (pvVar8 < pvVar10) {
      pvVar13 = pvVar10;
    }
    uVar1 = (longlong)pvVar13 + 1;
    if (uVar1 != 0) {
      if (0xfff < uVar1) {
        uVar9 = (longlong)pvVar13 + 0x28;
        if (uVar9 <= uVar1) {
                    /* WARNING: Subroutine does not return */
          FUN_1800191d0();
        }
        goto LAB_18004487e;
      }
      _Dst = (void *)FUN_180050418(uVar1);
    }
LAB_1800448a1:
    param_1[2] = uVar14;
    param_1[3] = (ulonglong)pvVar13;
    if (uVar3 < 0x10) {
      memcpy(_Dst,param_1,1);
    }
    else {
      pvVar13 = (void *)*param_1;
      memcpy(_Dst,pvVar13,1);
      puVar11 = auStack_1f8;
      if (uVar3 + 1 < 0x1000) goto LAB_1800448f1;
      puVar2 = (undefined8 *)((longlong)pvVar13 + -8);
      pvVar13 = (void *)((longlong)pvVar13 + (-8 - (longlong)*puVar2));
      if ((void *)0x1f < pvVar13) goto LAB_1800448ea;
      free((void *)*puVar2);
      puVar11 = auStack_1f8;
    }
  }
  else {
    uVar9 = 0x8000000000000027;
LAB_18004487e:
    lVar7 = FUN_180050418(uVar9);
    if (lVar7 != 0) {
      _Dst = (void *)(lVar7 + 0x27U & 0xffffffffffffffe0);
      *(longlong *)((longlong)_Dst - 8) = lVar7;
      goto LAB_1800448a1;
    }
LAB_1800448ea:
    pcVar4 = (code *)swi(0x29);
    (*pcVar4)(5);
    puVar11 = auStack_1f0;
LAB_1800448f1:
    *(undefined8 *)(puVar11 + -8) = 0x1800448f9;
    free(pvVar13);
  }
  *param_1 = (ulonglong)_Dst;
  param_1[2] = 0;
  puVar12 = puVar11;
LAB_18004490c:
  *(undefined1 **)(puVar12 + 0x88) = puVar12 + 0xa8;
  *(undefined8 *)(puVar12 + 0x90) = 0;
  *(undefined8 *)(puVar12 + 0x98) = 0x100;
  *(undefined ***)(puVar12 + 0x80) =
       std::
       _Fmt_iterator_buffer<std::back_insert_iterator<std::basic_string<char,std::char_traits<char>,std::allocator<char>_>_>,char,std::_Fmt_buffer_traits>
       ::vftable;
  *(ulonglong **)(puVar12 + 0xa0) = param_1;
  uVar5 = param_2[1];
  *(undefined8 *)(puVar12 + 0x40) = *param_2;
  *(undefined8 *)(puVar12 + 0x48) = uVar5;
  *(undefined8 *)(puVar12 + 0x50) = 0;
  *(undefined8 *)(puVar12 + 0x58) = 0;
  *(undefined1 **)(puVar12 + 0x60) = puVar12 + 0x80;
  uVar3 = param_3[1];
  *(ulonglong *)(puVar12 + 0x68) = *param_3;
  *(ulonglong *)(puVar12 + 0x70) = uVar3;
  *(undefined8 *)(puVar12 + 0x78) = 0;
  uVar5 = param_2[1];
  *(undefined8 *)(puVar12 + 0x30) = *param_2;
  *(undefined8 *)(puVar12 + 0x38) = uVar5;
  *(undefined8 *)(puVar12 + -8) = 0x180044990;
  FUN_18003c9f0((longlong *)(puVar12 + 0x30),(longlong *)(puVar12 + 0x40));
  *(undefined8 *)(puVar12 + -8) = 0x18004499d;
  FUN_180017a60((longlong)(puVar12 + 0x80));
  *(undefined ***)(puVar12 + 0x80) =
       std::
       _Fmt_iterator_buffer<std::back_insert_iterator<std::basic_string<char,std::char_traits<char>,std::allocator<char>_>_>,char,std::_Fmt_buffer_traits>
       ::vftable;
  if (*(longlong *)(puVar12 + 0x90) != 0) {
    *(undefined8 *)(puVar12 + -8) = 0x1800449be;
    FUN_180017a60((longlong)(puVar12 + 0x80));
  }
  *(undefined8 *)(puVar12 + -8) = 0x1800449d2;
  return param_1;
}

