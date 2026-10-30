// Embedding Zorba in a C++ program: compile a query, bind an external
// variable, iterate over the result items, and catch a static error.
#include <iostream>

#include <zorba/store_manager.h>
#include <zorba/zorba.h>
#include <zorba/zorba_exception.h>

using namespace zorba;

int main() {
  void* store = StoreManager::getStore();
  Zorba* engine = Zorba::getInstance(store);
  {
    XQuery_t query = engine->compileQuery(
        "declare variable $n external;\n"
        "for $i in 1 to $n return $i * $i");
    query->getDynamicContext()->setVariable(
        "n", engine->getItemFactory()->createInteger(5));

    Iterator_t it = query->iterator();
    it->open();
    Item item;
    while (it->next(item)) {
      std::cout << item.getStringValue() << '\n';
    }
    it->close();

    try {
      engine->compileQuery("for $x in (1, 2, 3)");
    } catch (ZorbaException const& e) {
      std::cout << "error " << e.diagnostic().qname().localname() << '\n';
    }
  }
  engine->shutdown();
  StoreManager::shutdownStore(store);
  return 0;
}
