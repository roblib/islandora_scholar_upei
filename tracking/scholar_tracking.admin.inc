<?php

function islandora_scholar_tracking_admin_form($form_state) {

  $form['tracking']['page'] = array(
    '#type' => 'fieldset',
    '#title' => t('Pages'),
  );

  $form['tracking']['page']['clear_page_by_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('PID'),
  );

  $form['tracking']['page']['clear_page_by_pid_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clear page count'),
  );

  $form['tracking']['page']['clear_page_all'] = array(
    '#type' => 'submit',
    '#value' => t('Clear all page counts'),
  );

  $form['tracking']['ds'] = array(
    '#type' => 'fieldset',
    '#title' => t('Datastreams'),
  );

  $form['tracking']['ds']['clear_ds_by_pid'] = array(
    '#type' => 'textfield',
    '#title' => t('PID'),
  );

  $form['tracking']['ds']['clear_ds_by_pid_submit'] = array(
    '#type' => 'submit',
    '#value' => t('Clear datastream count'),
  );

  $form['tracking']['ds']['clear_ds_all'] = array(
    '#type' => 'submit',
    '#value' => t('Clear all datastream counts'),
  );

  return $form;
}

function islandora_scholar_tracking_admin_form_submit($form, &$form_state) {
  $button = $form_state['clicked_button']['#id'];
  $result = '';
  switch ($button) {
    case 'edit-clear-page-by-pid-submit':
      $pid = $form_state['values']['clear_page_by_pid'];
      $result = islandora_scholar_clear_tracking('islandora_scholar_page_tracking', $pid);
      if ($result) {
        drupal_set_message(t("Page tracking for PID '@pid' deleted!", array('@pid' => $pid)));
      }
      break;
    case 'edit-clear-page-all':
      $result = islandora_scholar_clear_tracking('islandora_scholar_page_tracking');
      if ($result) {
        drupal_set_message(t('All page tracking deleted!'));
      }
      break;
    case 'edit-clear-ds-by-pid-submit':
      $pid = $form_state['values']['clear_ds_by_pid'];
      $result = islandora_scholar_clear_tracking('islandora_scholar_ds_tracking', $pid);
      if ($result) {
        drupal_set_message(t("Datastream tracking for PID '@pid' deleted!", array('@pid' => $pid)));
      }
      break;
    case 'edit-clear-ds-all':
      $result = islandora_scholar_clear_tracking('islandora_scholar_ds_tracking');
      if ($result) {
        drupal_set_message(t('All datastream tracking deleted!'));
      }
      break;
  }
//  drupal_set_message('Button: ' . implode(',', $button));
//  islandora_scholar_clear_tracking('islandora_scholar_page_tracking');
}

function islandora_scholar_clear_tracking($table, $pid = NULL) {
  $result = '';
  if ($pid == NULL) {
    $result = db_query("TRUNCATE TABLE %s", $table);
  }
  else {
    $result = db_query("DELETE FROM {%s} WHERE pid = '%s'", $table, $pid);
  }
  return $result;
}