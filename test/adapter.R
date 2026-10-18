library(licensebox)
r <- jsonlite::fromJSON(paste(readLines(file("stdin"),warn=FALSE),collapse="\n"),simplifyVector=FALSE)
run <- function(r) {
 if(r$op=="derive")return(licensebox_derive_binding(r$product,r$source,r$raw_id))
 if(r$op=="native")return(licensebox_native_binding(r$product))
 if(r$op=="verify")return(licensebox_verify_lease(r$token,r$product,r$keys,r$now))
 c <- licensebox_client(r$product,r$keys,version="conformance",server=r$server,directory=r$directory,ca_bundle=r$ca_bundle,clock=function()r$now,machine_binding=function()r$binding,on_sign_in=function(url,code)NULL)
 on.exit(licensebox_close(c))
 switch(r$op,heartbeat=licensebox_heartbeat(c,if(is.null(r$module))"core" else r$module),`heartbeat-repeat`={licensebox_heartbeat(c,"core");licensebox_heartbeat(c,"core")},`heartbeat-after-require`={licensebox_require(c,"core");licensebox_heartbeat(c,"core")},lease=licensebox_lease(c),require=licensebox_require(c,if(is.null(r$module))"core" else r$module),`repeat`={licensebox_require(c,"core");licensebox_require(c,"core")},status=licensebox_status(c),sign_in=licensebox_sign_in(c,3),logout=licensebox_logout(c),modules=licensebox_modules(c),requests=licensebox_requests(c),request=licensebox_request_access(c,c("export"),"café – test"),`access-link`=licensebox_access_link(c),record={licensebox_record(c,"export",TRUE,.125);licensebox_record(c,"export",FALSE,.025);NULL},stop("unknown operation"))
}
a <- tryCatch(list(ok=TRUE,value=run(r)),licensebox_error=function(e)list(ok=FALSE,error=e$code))
cat(jsonlite::toJSON(a,auto_unbox=TRUE,null="null",force=TRUE),"\n")
