<?php

/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
?>
<div id="clear-block">

  <h3><?php print t('Labelled view'); ?></h3>
  
  <?php print theme('table', $headers, $rows); ?>
  
  <h3><?php print t('Citation view'); ?></h3>
  
  <?php print $citation ?>

</div>