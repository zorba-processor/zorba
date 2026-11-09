### Breaking changes

- Restrict `zorba::SmartPtr<T>::operator=(SmartPtr<otherT> const&)` to types where `otherT*` converts implicitly to `T*`. Assigning a `SmartPtr` of a base or unrelated type to a `SmartPtr<T>` no longer compiles; it used to apply an unchecked `static_cast`. Embedders that downcast this way must cast the raw pointer explicitly, for example `derived = static_cast<Derived*>(base.get());`.
