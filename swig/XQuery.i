
%{  // start Implementation

class Iterator 
{
protected:
  zorba::Iterator_t theIterator;

public:
  Iterator() {}
  Iterator(const Iterator& anIterator) : theIterator(anIterator.theIterator) {}
  Iterator(zorba::Iterator_t anIterator) : theIterator(anIterator) {}
  void open()            { theIterator->open(); }
  bool next(Item& aItem);
  void close()           { theIterator->close(); }
  void destroy()         { theIterator = 0; }
}; // class Iterator


class ResultIterator : public Iterator
{
public:
  ResultIterator() {}
  ResultIterator(const ResultIterator& aResultIterator) 
    : Iterator(dynamic_cast<const Iterator&>(aResultIterator))   {}
  ResultIterator(zorba::ResultIterator_t aResultIterator)
    : Iterator(zorba::Iterator_t(aResultIterator.get())) {}
}; // class ResultIterator


class Item 
{
  friend class Iterator;
  friend class ResultIterator;
  friend class StaticContext;
  friend class DocumentURIResolver;

private:
  zorba::Item theItem;

public:
  Item() {}
  Item(const Item& aItem) : theItem(aItem.theItem) {}
  Item(const zorba::Item& aZItem) : theItem(aZItem) {}

  static Item createEmptyItem() 
  { return Item(); }

  std::string getStringValue() const 
  { return 		std::string(theItem.getStringValue().c_str()); }
  
  std::string serialize() const 
  {
    std::stringstream lStream; 
    Zorba_SerializerOptions_t lOptions; 
    zorba::Serializer_t lSerializer = zorba::Serializer::createSerializer(lOptions); 
    zorba::SingletonItemSequence lSequence(theItem); 
    lSerializer->serialize((zorba::Serializable*)&lSequence, lStream); 
    return lStream.str();
  }
  
  Item 	getAtomizationValue () const
  { return Item(theItem.getAtomizationValue()); }
  
  Iterator getAttributes () const
  { return Iterator(theItem.getAttributes()); }
  
  bool 	getBooleanValue () const
  { return theItem.getBooleanValue(); }

  Iterator 	getChildren () const
  { return Iterator(theItem.getChildren()); }

  Item 	getEBV () const
  { return Item(theItem.getEBV()); }

  int 	getIntValue () const
  { return theItem.getIntValue(); }
  
  std::string 	getLocalName () const
  { return std::string(theItem.getLocalName().c_str()); }

  std::string 	getNamespace () const
  { return std::string(theItem.getNamespace().c_str()); }

  bool 	getNodeName (Item &aNodeName) const
  { return theItem.getNodeName( aNodeName.theItem ); }

  std::string	getPrefix () const
  { return std::string(theItem.getPrefix().c_str()); }

  Item 	getType () const
  { return Item( theItem.getType() ); }

  unsigned int 	getUnsignedIntValue () const
  { return theItem.getUnsignedIntValue(); }

  bool 	isAtomic () const
  { return theItem.isAtomic(); }

  bool 	isNaN () const
  { return theItem.isNaN(); }

  bool 	isNode () const
  { return theItem.isNode(); }

  bool 	isNull () const
  { return theItem.isNull(); }

  bool 	isPosOrNegInf () const
  { return theItem.isPosOrNegInf(); }
}; // class Item


class XQuery 
{
private:
  zorba::XQuery_t theQuery;

public:
  XQuery() {}
  XQuery(const XQuery& aXQuery) : theQuery(aXQuery.theQuery) {}
  XQuery(zorba::XQuery_t aQuery) : theQuery(aQuery) {}

  std::string execute()
  {
    std::stringstream lStream;
    lStream << theQuery;
    return lStream.str();
  }

  void setVariableAsDocument(const std::string& aVarName, 
                             const std::string& aURL, 
                             const std::string& aContent)
  {
    std::auto_ptr<std::istream> lDocStream(new std::stringstream(aContent));
  	zorba::DynamicContext* lCtx = theQuery->getDynamicContext();

  	lCtx->setVariableAsDocument(aVarName, aURL, lDocStream);
   }

  std::string printPlanAsXML()
  {
    std::ostringstream lStream;
    theQuery->printPlan(lStream);
    return lStream.str();
  }

  std::string printPlanAsDOT()
  {
    std::ostringstream lStream;
    theQuery->printPlan(lStream, true);
    return lStream.str();
  }

#ifdef SWIGPYTHON
  void executeSAX(SAX2ContentHandlerProxy* contentHandlerProxy)
  { theQuery->executeSAX(contentHandlerProxy); }
#endif

  void destroy() { theQuery = 0; }
  ResultIterator iterator() { return ResultIterator(theQuery->iterator()); }
}; // class XQuery


// remaining method definitions from Iterator
bool Iterator::next(Item& aItem)
{ return theIterator->next(aItem.theItem); }


%}  // end   Implementation


// Interfaces

class Item 
{
public: 
  static Item createEmptyItem();
  std::string getStringValue() const;
  std::string serialize() const;
  Item 	getAtomizationValue () const;
  Iterator 	getAttributes () const;
  bool 	getBooleanValue () const;
  Iterator 	getChildren () const;
  Item 	getEBV () const;
  int 	getIntValue () const;
  std::string 	getLocalName () const;
  std::string 	getNamespace () const;
  bool 	getNodeName (Item &aNodeName) const;
  std::string 	getPrefix () const;
  Item 	getType () const;
  unsigned int 	getUnsignedIntValue () const;
  bool 	isAtomic () const;
  bool 	isNaN () const;
  bool 	isNode () const;
  bool 	isNull () const;
  bool 	isPosOrNegInf () const;
}; // class Item


class Iterator 
{
public:
  void open();
  bool next(Item&);
  void close();
  void destroy();
}; // class Iterator


class ResultIterator : public Iterator
{
}; // class ResultIterator


class XQuery 
{
public:
  std::string execute();
  std::string printPlanAsXML();
  std::string printPlanAsDOT();

#ifdef SWIGPYTHON
  void executeSAX(SAX2ContentHandlerProxy* contentHandlerProxy);
#endif

  void destroy();
  ResultIterator iterator();
  void setVariableAsDocument(const std::string& aVarName, 
                             const std::string& aURL, 
                             const std::string& aContent);
}; // class XQuery
