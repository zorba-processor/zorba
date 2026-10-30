(: XMark Q10: persons grouped by interest category, restructured :)
declare variable $auction as document-node() external;

for $cat in distinct-values($auction/site/people/person/profile/interest/@category)
let $group := for $p in $auction/site/people/person
              where $p/profile/interest/@category = $cat
              return <personne>
                <statistiques>
                  <sexe>{$p/profile/gender/text()}</sexe>
                  <age>{$p/profile/age/text()}</age>
                  <education>{$p/profile/education/text()}</education>
                  <revenu>{fn:data($p/profile/@income)}</revenu>
                </statistiques>
                <coordonnees>
                  <nom>{$p/name/text()}</nom>
                  <rue>{$p/address/street/text()}</rue>
                  <ville>{$p/address/city/text()}</ville>
                  <pays>{$p/address/country/text()}</pays>
                  <reseau>
                    <courrier>{$p/emailaddress/text()}</courrier>
                    <pagePerso>{$p/homepage/text()}</pagePerso>
                  </reseau>
                </coordonnees>
                <cartePaiement>{$p/creditcard/text()}</cartePaiement>
              </personne>
return <categorie>{<id>{$cat}</id>, $group}</categorie>
