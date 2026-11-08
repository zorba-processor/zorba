(: Array positions outside the array raise JNUP0016 when the update is
   created (JSONiq 1.0, "Primary updating expressions"). :)
declare namespace jerr = "http://jsoniq.org/errors";

try {
  copy $a := [ 1, 2, 3 ]
  modify replace value of json $a(0) with 9
  return "no error: replace at 0"
} catch jerr:JNUP0016 { "replace at 0" },

try {
  copy $a := [ 1, 2, 3 ]
  modify replace value of json $a(-1) with 9
  return "no error: replace at -1"
} catch jerr:JNUP0016 { "replace at -1" },

try {
  copy $a := [ 1, 2, 3 ]
  modify replace value of json $a(4) with 9
  return "no error: replace at size + 1"
} catch jerr:JNUP0016 { "replace at size + 1" },

try {
  copy $a := [ ]
  modify replace value of json $a(1) with 9
  return "no error: replace in empty array"
} catch jerr:JNUP0016 { "replace in empty array" },

try {
  copy $a := [ ]
  modify delete json $a(1)
  return "no error: delete in empty array"
} catch jerr:JNUP0016 { "delete in empty array" },

try {
  copy $a := [ 1, 2, 3 ]
  modify insert json 9 into $a at position 0
  return "no error: insert at 0"
} catch jerr:JNUP0016 { "insert at 0" },

try {
  copy $a := [ ]
  modify insert json 9 into $a at position 2
  return "no error: insert at size + 2"
} catch jerr:JNUP0016 { "insert at size + 2" }
