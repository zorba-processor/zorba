(: A quantifier may follow $ (QT3 re01001-re01003). :)
(fn:matches('alpha', 'alp$?ha'),
 fn:matches('alpha', 'alp${2,4}ha'),
 fn:matches('alpha', 'alpha$+') and not(fn:matches('alphax', 'alpha$+')),
 fn:matches('alpha', 'alpha$*'))
