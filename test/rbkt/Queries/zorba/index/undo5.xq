import module namespace u = "http://zorba.io/unique-index" at "undo4.xqlib";

import module namespace dml = "http://zorba.io/modules/store/static/collections/dml";
import module namespace idml = "http://zorba.io/modules/store/static/indexes/dml";

declare namespace zerr = "http://zorba.io/errors";
declare namespace err = "http://www.w3.org/2005/xqt-errors";

u:create-db();

dml:insert($u:c1, <person id="1"><car make="honda"/><car make="vw"/></person>);

dml:insert($u:c1, <person id="2"><car make="vw"/></person>);

dml:insert($u:c2, <person id="1"/>);

(: The refresh of f:PersonCars removes both keys of person 1, then f:PersonId fails. :)
try
{{
  (
    delete nodes dml:collection($u:c1)[1]/car,

    dml:insert($u:c2, <person id="1"/>)
  );
  ()
}}
catch *
{
  <exception>{$err:code}</exception>
}
,
idml:probe-index-point-general($u:PersonCars, "honda")
,
idml:probe-index-point-general($u:PersonCars, "vw")
,
try
{
  idml:probe-index-point-value($u:PersonCars, "vw")/.
}
catch *
{
  <exception>{$err:code}</exception>
}
