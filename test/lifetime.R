library(licensebox)
d <- tempfile();dir.create(d)
c <- licensebox_client("demo",list(`test-key`="A6EHv_POEL4dcN0Y50vAmWfk1jCbpQ1fHdyGZBJVMbg"),directory=d)
stopifnot(inherits(tryCatch(licensebox_status(c),licensebox_error=function(e)e),"licensebox_error"))
licensebox_close(c)
stopifnot(inherits(try(licensebox_status(c),silent=TRUE),"try-error"))
for(i in 1:100){x <- licensebox_client("demo",list(`test-key`="A6EHv_POEL4dcN0Y50vAmWfk1jCbpQ1fHdyGZBJVMbg"),directory=d);licensebox_close(x)}
gc()

c <- licensebox_client("demo",list(`test-key`="A6EHv_POEL4dcN0Y50vAmWfk1jCbpQ1fHdyGZBJVMbg"),directory=tempfile(),server="http://127.0.0.1:1",machine_binding=function()stop("expected callback failure"))
stopifnot(inherits(try(licensebox_sign_in(c,.1),silent=TRUE),"try-error"))
licensebox_close(c);licensebox_close(c)
