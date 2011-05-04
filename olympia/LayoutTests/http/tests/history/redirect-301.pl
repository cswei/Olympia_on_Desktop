#!/usr/bin/perl
# Script to generate a 301 HTTP redirect

print "Status: 301 Moved Permanently\r\n";
print "Location: resources/redirect-target.html#1\r\n";
print "Content-type: text/html\r\n";
print "\r\n";

print <<HERE_DOC_END
<html>
<head>
<title>301 Redirect</title>
<script>
if (window.layoutTestController) {
    layoutTestController.keepWebHistory();
    layoutTestController.waitUntilDone();
}
</script>

<body>This page is a 301 redirect.</body>
</html>
HERE_DOC_END
