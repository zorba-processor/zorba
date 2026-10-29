<xqdoc:xqdoc xmlns:xqdoc="http://www.xqdoc.org/1.0">
  <xqdoc:control>
    <xqdoc:date/>
    <xqdoc:version>1.0</xqdoc:version>
  </xqdoc:control>
  <xqdoc:module type="library">
    <xqdoc:uri>http://zorba-tests.example.org/colldecls</xqdoc:uri>
    <xqdoc:name>colldecls.xqlib</xqdoc:name>
    <xqdoc:custom tag="namespaces">
      <xqdoc:namespace prefix="an" uri="" isSchema="false"/>
      <xqdoc:namespace prefix="cd" uri="http://zorba-tests.example.org/colldecls" isSchema="false"/>
      <xqdoc:namespace prefix="ddl" uri="http://zorba.io/modules/store/static/collections/ddl" isSchema="false"/>
      <xqdoc:namespace prefix="dml" uri="http://zorba.io/modules/store/static/collections/dml" isSchema="false"/>
      <xqdoc:namespace prefix="functx" uri="http://www.functx.com" isSchema="false"/>
      <xqdoc:namespace prefix="idml" uri="http://zorba.io/modules/store/static/indexes/dml" isSchema="false"/>
      <xqdoc:namespace prefix="ma" uri="http://zorba-tests.example.org/modules/alpha" isSchema="false"/>
      <xqdoc:namespace prefix="mb" uri="http://zorba-tests.example.org/modules/beta" isSchema="false"/>
    </xqdoc:custom>
  </xqdoc:module>
  <xqdoc:imports>
    <xqdoc:import type="library">
      <xqdoc:uri>http://www.functx.com</xqdoc:uri>
    </xqdoc:import>
    <xqdoc:import type="library">
      <xqdoc:uri>http://zorba-tests.example.org/modules/alpha</xqdoc:uri>
    </xqdoc:import>
    <xqdoc:import type="library">
      <xqdoc:uri>http://zorba-tests.example.org/modules/beta</xqdoc:uri>
    </xqdoc:import>
    <xqdoc:import type="library">
      <xqdoc:uri>http://zorba.io/modules/store/static/collections/ddl</xqdoc:uri>
    </xqdoc:import>
    <xqdoc:import type="library">
      <xqdoc:uri>http://zorba.io/modules/store/static/collections/dml</xqdoc:uri>
    </xqdoc:import>
    <xqdoc:import type="library">
      <xqdoc:uri>http://zorba.io/modules/store/static/indexes/dml</xqdoc:uri>
    </xqdoc:import>
  </xqdoc:imports>
  <xqdoc:variables>
    <xqdoc:variable>
      <xqdoc:uri>cd:orders</xqdoc:uri>
      <xqdoc:invoked arity="1">
        <xqdoc:uri>http://www.w3.org/2001/XMLSchema</xqdoc:uri>
        <xqdoc:name>QName</xqdoc:name>
      </xqdoc:invoked>
    </xqdoc:variable>
    <xqdoc:variable>
      <xqdoc:uri>cd:by-customer</xqdoc:uri>
      <xqdoc:invoked arity="1">
        <xqdoc:uri>http://www.w3.org/2001/XMLSchema</xqdoc:uri>
        <xqdoc:name>QName</xqdoc:name>
      </xqdoc:invoked>
    </xqdoc:variable>
  </xqdoc:variables>
  <xqdoc:collections>
    <xqdoc:collection>
      <xqdoc:name>orders</xqdoc:name>
      <xqdoc:type>element(order)*</xqdoc:type>
      <xqdoc:comment>
        <xqdoc:description> An ordered collection of typed items.
</xqdoc:description>
      </xqdoc:comment>
      <xqdoc:annotations>
        <xqdoc:annotation prefix="an" namespace="" localname="ordered" value=""/>
      </xqdoc:annotations>
    </xqdoc:collection>
    <xqdoc:collection>
      <xqdoc:name>log</xqdoc:name>
      <xqdoc:comment>
        <xqdoc:description> An ordered collection without a type.
</xqdoc:description>
      </xqdoc:comment>
      <xqdoc:annotations>
        <xqdoc:annotation prefix="an" namespace="" localname="ordered" value=""/>
      </xqdoc:annotations>
    </xqdoc:collection>
    <xqdoc:collection>
      <xqdoc:name>items</xqdoc:name>
      <xqdoc:type>element(item)*</xqdoc:type>
      <xqdoc:comment>
        <xqdoc:description> An unordered collection of typed items.
</xqdoc:description>
      </xqdoc:comment>
    </xqdoc:collection>
    <xqdoc:collection>
      <xqdoc:name>misc</xqdoc:name>
      <xqdoc:comment>
        <xqdoc:description> An unordered collection without a type.
</xqdoc:description>
      </xqdoc:comment>
    </xqdoc:collection>
  </xqdoc:collections>
  <xqdoc:indexes>
    <xqdoc:index>
      <xqdoc:name>by-customer</xqdoc:name>
      <xqdoc:source>
        <xqdoc:uri>http://zorba-tests.example.org/colldecls</xqdoc:uri>
        <xqdoc:name>orders</xqdoc:name>
      </xqdoc:source>
      <xqdoc:comment>
        <xqdoc:description> A unique automatic index on the customer of each order.
</xqdoc:description>
      </xqdoc:comment>
      <xqdoc:annotations>
        <xqdoc:annotation prefix="an" namespace="" localname="automatic" value=""/>
        <xqdoc:annotation prefix="an" namespace="" localname="unique" value=""/>
        <xqdoc:annotation prefix="an" namespace="" localname="value-equality" value=""/>
      </xqdoc:annotations>
    </xqdoc:index>
    <xqdoc:index>
      <xqdoc:name>by-customer-manual</xqdoc:name>
      <xqdoc:source>
        <xqdoc:uri>http://zorba-tests.example.org/colldecls</xqdoc:uri>
        <xqdoc:name>orders</xqdoc:name>
      </xqdoc:source>
      <xqdoc:comment>
        <xqdoc:description> A manually maintained index on the same key.
</xqdoc:description>
      </xqdoc:comment>
    </xqdoc:index>
  </xqdoc:indexes>
  <xqdoc:functions/>
</xqdoc:xqdoc>