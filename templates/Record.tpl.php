<br/>
<div>
  Place holder For Citation and Citation Form.
</div>
<br/>
<div>
  Place holder For Link
</div>
<?php
print theme_fieldset(array(
      '#title' => t("Technical Details"),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#children' => theme_table(array(
        array(
          'colspan' => 3,
          'data' => 'Technical Details'
        ),
        array(
          'data' => 'Mime Type'
          )), $rows
      ) . l('Edit Metadata', '') . '<br/>' . l('Manage Object', '')
    ));
?>
