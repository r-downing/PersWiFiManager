---
layout: page
title: API Reference

---

{% assign classes = site.apis | group_by: 'api-class' %}
{% for class in classes %}
# {{class.name}}
<ul style="font-family:monospace">
{% assign ugroups = class.items | group_by: 'api-group' %}
{% assign groups = ugroups | sort: 'name' %}
{% for group in groups %}
  <li>{{ group.name }}
    <ul>
    {% assign items = group.items | sort: 'title' %}
    {% for api in items %}
      <li><a href="{{ site.baseurl }}{{ api.url }}">{{ api.title }}</a></li>
    {% endfor %}
    </ul>
  </li>
{% endfor %}
</ul>

{% endfor %}
