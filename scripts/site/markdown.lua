-- Pandoc Lua filter: turns a built site page (HTML) into a Markdown page that
-- holds only the main content. Used by scripts/site/make_markdown.sh.
--
-- It drops the site chrome (table of contents, pager, header, footer, nav),
-- renders example figures as a labelled code block plus a result block,
-- replaces inline SVG diagrams by their accessible description, and rewrites
-- internal links from x.html to x.md.

local drop = { pager = true, toc = true, ["site-head"] = true,
               ["site-foot"] = true, bar = true, details = true }

local fence = { xq = "xquery", jq = "jsoniq", cpp = "cpp", xml = "xml",
                json = "json" }

local function has_class(el, set)
  for _, c in ipairs(el.classes) do
    if set[c] then return true end
  end
  return false
end

function Div(el)
  if has_class(el, drop) then return {} end
end

local function lang_of(figure)
  for _, c in ipairs(figure.classes) do
    local l = c:match("^lang%-(.+)$")
    if l then return fence[l] or l end
  end
end

local b64chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"

local function b64decode(data)
  data = data:gsub("[^" .. b64chars .. "]", "")
  local bits = data:gsub(".", function(x)
    local f = b64chars:find(x, 1, true) - 1
    local r = ""
    for i = 6, 1, -1 do
      r = r .. ((f % 2 ^ i - f % 2 ^ (i - 1) > 0) and "1" or "0")
    end
    return r
  end)
  return (bits:gsub("%d%d%d%d%d%d%d%d", function(x)
    local c = 0
    for i = 1, 8 do
      if x:sub(i, i) == "1" then c = c + 2 ^ (8 - i) end
    end
    return string.char(c)
  end))
end

local function svg_label(src)
  local b64 = src:match("^data:image/svg%+xml;base64,(.*)$")
  if not b64 then return nil end
  local label = b64decode(b64):match('aria%-label="([^"]*)"') or "diagram"
  label = label:gsub("&quot;", '"'):gsub("&lt;", "<"):gsub("&gt;", ">")
               :gsub("&amp;", "&")
  return label
end

function Image(el)
  local label = svg_label(el.src)
  if label then
    return pandoc.Emph({ pandoc.Str("Diagram: " .. label) })
  end
end

local function text_of(x)
  return pandoc.utils.stringify(x)
end

local function example(el)
  local out = {}
  local lang = lang_of(el)
  local langname, file
  for _, b in ipairs(el.caption.long) do
    for _, i in ipairs(b.content or {}) do
      if i.t == "Span" then langname = text_of(i.content) end
      if i.t == "Link" then file = text_of(i.content) end
    end
  end
  local label = { pandoc.Strong({ pandoc.Str(langname or "Example") }) }
  if file then
    label[#label + 1] = pandoc.Str(": ")
    label[#label + 1] = pandoc.Code(file)
  end
  out[#out + 1] = pandoc.Para(label)
  for _, b in ipairs(el.content) do
    if b.t == "CodeBlock" then
      b.classes = lang and { lang } or {}
      out[#out + 1] = b
    elseif b.t == "Div" and b.classes:includes("result") then
      local cmd, block
      pandoc.walk_block(b, {
        Code = function(c)
          if c.classes:includes("cmd") then cmd = c.text end
        end,
        CodeBlock = function(c)
          c.classes = {}
          block = c
        end,
      })
      local p = { pandoc.Strong({ pandoc.Str("Result") }) }
      if cmd then
        p[#p + 1] = pandoc.Str(": ")
        p[#p + 1] = pandoc.Code(cmd)
      end
      out[#out + 1] = pandoc.Para(p)
      if block then out[#out + 1] = block end
    end
  end
  return out
end

function Figure(el)
  if el.classes:includes("example") then return example(el) end
  -- any other figure (diagrams): keep the content, drop the wrapper
  return el.content
end

function CodeBlock(el)
  if el.classes:includes("shell") then
    el.classes = { "sh" }
  elseif el.classes:includes("code") or el.classes:includes("output") then
    el.classes = {}
  end
  return el
end

function Link(el)
  if el.target:match("^%a[%w+.-]*:") then return el end
  local page, frag = el.target:match("^([^#?]*)%.html(#?.*)$")
  if page then
    el.target = page .. ".md" .. frag
    return el
  end
end

-- Last pass: write every code block as a fenced block with an explicit
-- language, whatever pandoc's own list of highlighted languages knows.
function Pandoc(doc)
  doc.blocks = doc.blocks:walk({
    CodeBlock = function(el)
      local longest = 0
      for run in el.text:gmatch("`+") do
        if #run > longest then longest = #run end
      end
      local ticks = string.rep("`", math.max(3, longest + 1))
      local lang = el.classes[1] or ""
      return pandoc.RawBlock("markdown",
        ticks .. lang .. "\n" .. el.text .. "\n" .. ticks)
    end,
  })
  return doc
end
