---
layout: page
title: Updates
---

  <ul class="post-list">
    {% for p in site.posts %}
		{% assign post=p %}
		{% include post-snippet.html %}
    {% endfor %}
  </ul>