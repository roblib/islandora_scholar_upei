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
    $mods = $object->get_datastream_dissemination('MODS');
    $rnd = rand('1000', '9999');
    $mods_filename = file_directory_path() . '/mods' . $rnd . '.temp';
    $mods_handle = fopen($mods_filename, 'w');
    fwrite($mods_handle, $mods);
    fclose($mods_handle);
    $ris_filename = file_directory_path() . '/ris' . $rnd . '.temp';
    Bibutils::Convert($mods_filename, 'MODS', $ris_filename, 'RIS');
    $ris = file_get_contents($ris_filename);    
    echo url_decode(trim($ris));
  }
}