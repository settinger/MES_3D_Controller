try {
  Object.defineProperty(Element.prototype, "setAttributes", {
    value: function (attributes = {}) {
      for (let [key, value] of Object.entries(attributes)) {
        if (key.any("text", "textContent", "innerText", "innerHTML")) {
          if (SVGElement.prototype.isPrototypeOf(this)) {
            this.textContent = value;
          } else {
            this.innerHTML = value;
          }
        } else {
          this.setAttribute(key, value);
        }
      }
    },
  });
  Object.defineProperty(Element.prototype, "clear", {
    value: function () {
      while (this.firstChild) this.removeChild(this.firstChild);
    },
  });
  Object.defineProperty(Element.prototype, "appendHTML", {
    value: function (childTag = "div", childProps = {}) {
      const child = document.createElement(childTag);
      child.setAttributes(childProps);
      this.appendChild(child);
      return child;
    },
  });
  Object.defineProperty(SVGElement.prototype, "appendSVG", {
    value: function (childTag = "svg", childProps = {}) {
      const child = document.createElementNS(
        "http://www.w3.org/2000/svg",
        childTag
      );
      child.setAttributes(childProps);
      this.appendChild(child);
      return child;
    },
  });
} catch {
  throw "An error occurred while adding functions to native objects";
}

// Shortcut to create an HTML element and give it multiple attributes at once
const newHTML = (tag, props = {}) => {
  const node = document.createElement(tag);
  node.setAttributes(props);
  return node;
};

// Shortcut to create an SVG element and give it multiple attributes at once
const newSVG = (tag, props = {}) => {
  const node = document.createElementNS("http://www.w3.org/2000/svg", tag);
  node.setAttributes(props);
  return node;
};
