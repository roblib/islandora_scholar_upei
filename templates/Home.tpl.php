<br/>
<h4>Quick Search</h4>
<div id="ir-quick-search-form">
  <?php print drupal_get_form('scholar_search_form') ?>
</div>
<!--
<?php if ($browse_by_user || $browse_by_department): ?>
  <div>or</div>
  <br/>
  <?php if ($browse_by_user): ?>
    <h4>Browse by User</h4> 
    <div id="ir-browse-by-user-form">
      <?php print drupal_get_form('scholar_browse_by_user_form') ?>
    </div>
    <br/>
  <?php endif; ?>
  <?php if ($browse_by_department): ?>
    <h4>Browse by Department</h4>
    <br/>
    <div id="ir-browse-by-department">
      <?php print theme_item_list(array(l('Place Holder', 'fedorair/ir'))); ?>
    </div>
  <?php endif; ?>
<?php endif; ?>
-->