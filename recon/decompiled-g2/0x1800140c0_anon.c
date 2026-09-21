// PrismExport addr=0x1800140c0 name=anon ghidra=FUN_1800140c0 status=clean lines=18

undefined * FUN_1800140c0(void)

{
  if (*(int *)(*(longlong *)((longlong)ThreadLocalStoragePointer + (ulonglong)_tls_index * 8) + 0x34
              ) < DAT_18009c188) {
    FUN_180050874(&DAT_18009c188);
    if (DAT_18009c188 == -1) {
      FUN_18000fef0((undefined4 *)&DAT_18009c0c0);
      atexit(FUN_180054d30);
      _Init_thread_footer(&DAT_18009c188);
      return &DAT_18009c0c0;
    }
  }
  return &DAT_18009c0c0;
}

