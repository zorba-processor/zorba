### Fixed

- Fix `fn:environment-variable` and `fn:available-environment-variables`
  answering from the variables read so far after an environment variable that
  is not valid UTF-8 had raised `err:FOCH0001` (for example, when that error
  was caught): every call now raises the error while such a variable exists.
