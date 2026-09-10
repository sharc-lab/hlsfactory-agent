set rc [catch {source run.tcl} err]
if {$rc} { puts "BATCH_ERROR: $err" }
exit
