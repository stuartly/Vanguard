--TEST--
Phar front controller $_SERVER munging failure 2 tar-based
--SKIPIF--
<?php if (!extension_loaded("phar")) die("skip"); ?>
--ENV--
SCRIPT_NAME=/frontcontroller19.phar.php
REQUEST_URI=/frontcontroller19.phar.php/
PATH_INFO=/
--FILE_EXTERNAL--
files/frontcontroller10.phar.tar
--EXPECTF--
Fatal error: Uncaught exception 'PharException' with message 'Too many values passed to Phar::mungServer(), expecting an array of any of these strings: PHP_SELF, REQUEST_URI, SCRIPT_FILENAME, SCRIPT_NAME' in %sfrontcontroller19.phar.php:2
Stack trace:
#0 %sfrontcontroller19.phar.php(2): Phar::mungServer(Array)
#1 {main}
  thrown in %sfrontcontroller19.phar.php on line 2
