; ModuleID = 'calc'
source_filename = "calc"
target triple = "x86_64-unknown-linux-gnu"

; Function Attrs: nounwind readnone
declare { i64, i1 } @llvm.sadd.with.overflow.i64(i64, i64) #0

; Function Attrs: nounwind readnone
declare { i64, i1 } @llvm.ssub.with.overflow.i64(i64, i64) #0

; Function Attrs: nounwind readnone
declare { i64, i1 } @llvm.smul.with.overflow.i64(i64, i64) #0

declare void @overflow_fail(i64)

define i64 @f(i64, i64, i64, i64, i64, i64) {
entry:
  %6 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 1, i64 1)
  %7 = extractvalue { i64, i1 } %6, 0
  %8 = extractvalue { i64, i1 } %6, 1
  br i1 %8, label %over, label %safe

over:                                             ; preds = %entry
  tail call void @overflow_fail(i64 30)
  unreachable

safe:                                             ; preds = %entry
  %9 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 %7, i64 1)
  %10 = extractvalue { i64, i1 } %9, 0
  %11 = extractvalue { i64, i1 } %9, 1
  br i1 %11, label %over1, label %safe2

over1:                                            ; preds = %safe
  tail call void @overflow_fail(i64 27)
  unreachable

safe2:                                            ; preds = %safe
  %12 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 1, i64 %10)
  %13 = extractvalue { i64, i1 } %12, 0
  %14 = extractvalue { i64, i1 } %12, 1
  br i1 %14, label %over3, label %safe4

over3:                                            ; preds = %safe2
  tail call void @overflow_fail(i64 22)
  unreachable

safe4:                                            ; preds = %safe2
  %15 = tail call { i64, i1 } @llvm.smul.with.overflow.i64(i64 %13, i64 1)
  %16 = extractvalue { i64, i1 } %15, 0
  %17 = extractvalue { i64, i1 } %15, 1
  br i1 %17, label %over5, label %safe6

over5:                                            ; preds = %safe4
  tail call void @overflow_fail(i64 19)
  unreachable

safe6:                                            ; preds = %safe4
  ret i64 %16
}

attributes #0 = { nounwind readnone }
