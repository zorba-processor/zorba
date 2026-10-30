// Highlights the section of the page currently on screen in the side
// navigation, and keeps the navigation closed by default on small screens.
(function () {
  var toc = document.querySelector(".toc");
  if (!toc) return;
  if (window.matchMedia("(min-width: 901px)").matches) toc.open = true;

  var links = Array.prototype.slice.call(toc.querySelectorAll('a[href^="#"]'));
  var byId = {};
  links.forEach(function (a) { byId[a.getAttribute("href").slice(1)] = a; });
  var targets = links
    .map(function (a) { return document.getElementById(a.getAttribute("href").slice(1)); })
    .filter(Boolean);
  if (!("IntersectionObserver" in window) || !targets.length) return;

  var visible = {};
  var observer = new IntersectionObserver(function (entries) {
    entries.forEach(function (e) { visible[e.target.id] = e.isIntersecting; });
    var current = targets.find(function (t) { return visible[t.id]; });
    if (!current) return;
    links.forEach(function (a) { a.classList.remove("active"); });
    byId[current.id].classList.add("active");
  }, { rootMargin: "-70px 0px -60% 0px" });
  targets.forEach(function (t) { observer.observe(t); });
})();
