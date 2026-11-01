(: Without a root argument or a binding for $dir:root there is nothing to
 : read. :)
import module namespace dir = "http://zorba.io/modules/directory";

dir:list($dir:root)
