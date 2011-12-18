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
    $ris2 = <<<RIS
RT Journal
ID 11579
A1 Mazzone,P. J.
A1 Arroliga,A. C.
T1 Lung cancer: Preoperative pulmonary evaluation of the lung resection candidate
JF The American Journal of Medicine
JO Am.J.Med.
YR 2005
FD Jun
VO 118
IS 6
SP 578
OP 583
AB Lung resection provides the best chance of cure for individuals with early stage non-small cell lung cancer. Naturally, lung resection will lead to a decrease in lung function. The population that develops lung cancer often has concomitant lung disease and a reduced ability to tolerate further losses in lung function. The goal of the preoperative pulmonary assessment of individuals with resectable lung cancer is to identify those individuals whose short- and long-term morbidity and mortality would be unacceptably high if surgical resection were to occur. Pulmonary function measures such as the forced expiratory volume in 1 second and the diffusing capacity for carbon monoxide are useful predictors of postoperative outcome. In situations in which lung function is not normal, the prediction of postoperative lung function from preoperative results and the assessment of exercise capacity can be performed to further clarify risks. Published guidelines help to direct the order of testing, permitting us to offer resection to as many patients as possible.
NO JID: 0267200; 2004/12/07 [received]; 2004/12/22 [revised]; 2004/12/29 [accepted]; ppublish
PP United States
SN 0002-9343
AD Department of Pulmonary, Allergy, and Critical Care Medicine, The Cleveland Clinic Foundation.
AN 15922686
LA eng
SF Journal Article; AIM; IM
DO S0002-9343(05)00147-6 [pii]; 10.1016/j.amjmed.2004.12.024 [doi]
RIS;
    
    echo trim($ris2);
  }
}