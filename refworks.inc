<?php

/**
 * @file
 * 
 */
function scholar_refworks_add_form($pid = NULL) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'scholar', 'bibutils/Bibutils');
  if ($pid != NULL) {
    $object = new Fedora_Item($pid);
    $ds_array = $object->get_datastreams_list_as_array();
    if (array_key_exists('MODS', $ds_array)) {
    $mods = $object->get_datastream_dissemination('MODS');
    $rnd = rand('1000', '9999');
    $mods_filename = file_directory_path() . '/mods' . $rnd . '.temp';
    $mods_handle = fopen($mods_filename, 'w');
    fwrite($mods_handle, $mods);
    fclose($mods_handle);
    $ris_filename = file_directory_path() . '/ris' . $rnd . '.temp';
    Bibutils::Convert($mods_filename, 'MODS', $ris_filename, 'RIS');
    $ris = file_get_contents($ris_filename);    
    echo trim($ris);
    }
    else {
      drupal_set_message(t('Could not export to Refworks. MODS record not found for item @pid!', array('@pid' => $pid)), 'error');
    }
  }
}