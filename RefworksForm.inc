<?php

// $Id$

/**
 * @file
 *
 * Contains the hooks for rendering and processing Refworks Metadata.
 */

/**
 * Builds a drupal form for editing refworks metadata.
 * 
 * @param array $form_state
 * @param string $pid
 * 
 * @return array
 */
function scholar_refworks_form(array $form_state, $pid) {
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $object = new ObjectHelper();
  $refworks = $object->getStream($pid, 'refworks', 0);
  if (!isset($refworks)) {
    drupal_set_message(t('Error getting refworks metadata stream'), 'error');
    return NULL;
  }
  // Authors
  $xml = new SimpleXMLElement($refworks);
  $authors = '';
  foreach ($xml->xpath('//a1') as $author) {
    $authors .= $author . '; ';
  }
  // @todo User instead - implode('; ', $xml->xpath('//a1'));
  // Titles
  $titles = '';
  foreach ($xml->xpath('//t1') as $title) {
    $string = strip_tags($title->asXML());
    $string = html_entity_decode($string);
    $titles .= $string . ' ';
  }
  // JFS
  $jfs = '';
  foreach ($xml->xpath('//jf') as $jf) {
    $string = strip_tags($jf->asXML());
    $string = html_entity_decode($string);
    $jfs .= $string . ' ';
  }
  // A2S
  $a2 = '';
  foreach ($xml->xpath('//a2') as $a2) {
    $a2s .= $a2 . '; ';
  }
  // T2S
  $t2 = '';
  foreach ($xml->xpath('//t2') as $t2) {
    $t2s .= $t2 . ' ';
  }
  // JOS
  $jos = '';
  foreach ($xml->xpath('//jo') as $jo) {
    $string = strip_tags($jo->asXML());
    $string = html_entity_decode($string);
    $jos .= $string . ' ';
  }
  // YRS
  $yrs = '';
  foreach ($xml->xpath('//yr') as $yr) {
    $yrs .= $yr . ' ';
  }
  // FDS
  $fds = '';
  foreach ($xml->xpath('//fd') as $fd) {
    $fds .= $fd . ' ';
  }
  // $vos = '';
  foreach ($xml->xpath('//vo') as $vo) {
    $vos .= $vo . ' ';
  }
  // Actual form
  $form = array();
  $form['rt'] = array(
    '#type' => 'select',
    '#default_value' => $xml->xpath('//rt'),
    '#title' => t('Type - rt'),
    '#options' => array(
      'Abstract' => 'Abstract',
      'Artwork' => 'Artwork',
      'Bills/Resolutions' => 'Bills/Resolutions',
      'Book, Edited' => 'Book, Edited',
      'Book, Section' => 'Book, Section',
      'Book, Whole' => 'Book, Whole',
      'Case/Court Decisions' => 'Case/Court Decisions',
      'Computer Program' => 'Computer Program',
      'Conference Proceedings' => 'Conference Proceedings',
      'Dissertation/Thesis' => 'Dissertation/Thesis',
      'Dissertation/Thesis, Unpublished' => 'Dissertation/Thesis, Unpublished',
      'Generic' => 'Generic',
      'Grant' => 'Grant',
      'Hearing' => 'Hearing',
      'Journal Article' => 'Journal Article',
      'Journal, Electronic' => 'Journal, Electronic',
      'Laws/Statutes' => 'Laws/Statutes',
      'Magazine Article' => 'Magazine Article',
      'Map' => 'Map',
      'Monograph' => 'Monograph',
      'Motion Picture' => 'Motion Picture',
      'Music Score' => 'Music Score',
      'Newspaper Article' => 'Newspaper Article',
      'Online Discussion Forum/Blogs' => 'Online Discussion Forum/Blogs',
      'Patent' => 'Patent',
      'Personal Communication' => 'Personal Communication',
      'Report' => 'Report',
      'Sound Recording' => 'Sound Recording',
      'Unpublished Material' => 'Unpublished Material',
      'Video/DVD' => 'Video/DVD',
      'Web Page' => 'Web Page'
    )
  );
  $form['pid'] = array(
    '#type' => 'hidden',
    '#value' => $pid,
  );
  $form['sr'] = array(
    '#type' => 'select',
    '#options' => array(
      'Print' => 'Print',
      'Electronic' => 'Electronic'
    ),
    '#default_value' => $xml->xpath('//sr'),
    '#title' => t('Source Type - sr')
  );
  $form['a1'] = array(
    '#type' => 'textarea',
    '#required' => TRUE,
    '#default_value' => $authors,
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Authors - a1')
  );
  $form['t1'] = array(
    '#type' => 'textarea',
    '#required' => TRUE,
    '#default_value' => $titles,
    '#title' => t('Primary Title - t1')
  );
  $form['jf'] = array(
    '#type' => 'textarea',
    '#default_value' => $jfs,
    '#title' => t('Periodical Full - jf')
  );
  $form['a2'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($a2s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Secondary Authors - a2')
  );
  $form['t2'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($t2s),
    '#title' => t('Secondary Title - t2')
  );
  $form['jo'] = array(
    '#type' => 'textarea',
    '#default_value' => $jos,
    '#title' => t('Periodical Abbrev - jo')
  );
  $form['yr'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($yrs),
    '#title' => t('Publication Year - yr')
  );
  $form['fd'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($fds),
    '#title' => t('Publication Date (free form) - fd')
  );
  $form['vo'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($vos),
    '#title' => t('Volume - vo')
  );
  $iss = '';
  foreach ($xml->xpath('//is') as $is) {
    $iss .= $is . ' ';
  }
  $form['is'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($iss),
    '#title' => t('Issue - is')
  );
  $sps = '';
  foreach ($xml->xpath('//sp') as $sp) {
    $sps .= $sp . ' ';
  }
  $form['sp'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($sps),
    '#title' => t('Start Page - sp')
  );
  $ops = '';
  foreach ($xml->xpath('//op') as $op) {
    $ops .= $op . ' ';
  }
  $form['refworks-op'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($ops),
    '#title' => t('End Page - op')
  );
  $pb = '';
  foreach ($xml->xpath('//pb') as $pb) {
    $pbs .= $pb . ' ';
  }
  $form['pb'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($pbs),
    '#title' => t('Publisher - pb')
  );
  $pps = '';
  foreach ($xml->xpath('//pp') as $pp) {
    $pps .= $pp . ' ';
  }
  $form['pp'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($pps),
    '#title' => t('Place of Publication - pp')
  );
  $abs = '';
  foreach ($xml->xpath('//ab') as $ab) {
    $string = strip_tags($ab->asXML());
    $string = html_entity_decode($string);
    $abs .= $string . ' ';
  }
  $form['ab'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($abs),
    '#title' => t('Abstract - ab')
  );
  $nos = '';
  foreach ($xml->xpath('//no') as $no) {
    $string = strip_tags($no->asXML());
    $string = html_entity_decode($string);
    $nos .= $string . ' ';
  }
  $form['no'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($nos),
    '#title' => t('Notes - no')
  );
  $form['status'] = array(
    '#type' => 'select',
    '#options' => array(
      'PUBLISHED' => 'Published',
      'NO_OBJ' => 'No Text Available',
      'POST-PRINT' => 'Post-Print',
      'PRE-PRINT' => 'Pre-Print',
      'OTHER' => 'Other/Unpublished'
    ),
    '#default_value' => $xml->xpath('//status'),
    '#title' => t('Status')
  );
  $form['usage'] = array(
    '#type' => 'select',
    '#options' => array(
      'Contact Author' => 'Contact Author',
      'Contact Publisher' => 'Contact Publisher',
      'Creative Commons License by <a href="http://creativecommons.org/licenses/by-nc-nd/3.0/deed.en_CA" target="_blank">(by-nc-nd)</a>' => 'Creative Commons'
    ),
    '#default_value' => $xml->xpath('//usage'),
    '#title' => t('Usage')
  );
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Update',
    '#suffix' => 'Less frequently used elements below<br /><hr />'
  );
  $k1s = '';
  foreach ($xml->xpath('//k1') as $k1) {
    $string = strip_tags($k1->asXML());
    $string = html_entity_decode($string);
    $k1s .= $string . '; ';
  }
  $form['k1'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($k1s),
    '#title' => t('Keywords/Descriptors - k1')
  );
  $ed = '';
  foreach ($xml->xpath('//ed') as $ed) {
    $eds .= $ed . ' ';
  }
  $form['ed'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($eds),
    '#title' => t('Edition - ed')
  );
  $a3s = '';
  foreach ($xml->xpath('//a3') as $a3) {
    $a3s .= $a3 . '; ';
  }
  $form['a3'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($a3s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Tertiary Authors - a3')
  );
  $a4s = '';
  foreach ($xml->xpath('//a4') as $a4) {
    $a4s .= $a4 . '; ';
  }
  $form['a4'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($a4s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Quaternary Authors - a4')
  );
  $a5s = '';
  foreach ($xml->xpath('//a5') as $a5) {
    $a5s .= $a5 . '; ';
  }
  $form['a5'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($a5s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Quinary Authors - a5')
  );
  $t3s = '';
  foreach ($xml->xpath('//t3') as $t3) {
    $t3s .= $t3 . ' ';
  }
  $form['t3'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($t3s),
    '#title' => t('Tertiary Title - t3')
  );
  $sns = '';
  foreach ($xml->xpath('//sn') as $sn) {
    $sns .= $sn . ' ';
  }
  $form['sn'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($sns),
    '#title' => t('ISSN/ISBN - sn')
  );
  $avs = '';
  foreach ($xml->xpath('//av') as $av) {
    $avs .= $av . ' ';
  }
  $form['av'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($avs),
    '#title' => t('Availiablity - av')
  );
  $ads = '';
  foreach ($xml->xpath('//ad') as $ad) {
    $ads .= $ad . ' ';
  }
  $form['ad'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($ads),
    '#title' => t('Author Address - ad')
  );
  $ans = '';
  foreach ($xml->xpath('//an') as $an) {
    $ans .= $an . ' ';
  }
  $form['an'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($ans),
    '#title' => t('Accession Number - an')
  );
  $las = '';
  foreach ($xml->xpath('//la') as $la) {
    $las .= $la . ' ';
  }
  $form['la'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($las),
    '#title' => t('Language - la')
  );
  $cls = '';
  foreach ($xml->xpath('//cl') as $cl) {
    $cls .= $cl . ' ';
  }
  $form['cl'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($cls),
    '#title' => t('Classification - cl')
  );
  $sfs = '';
  foreach ($xml->xpath('//sf') as $sf) {
    $sfs .= $sf . ' ';
  }
  $form['sf'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($sfs),
    '#title' => t('Subfile/Database - sf')
  );
  $ots = '';
  foreach ($xml->xpath('//ot') as $ot) {
    $ots .= $ot . ' ';
  }
  $form['ot'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($ots),
    '#title' => t('Original Foreign Title - ot')
  );
  $lks = '';
  foreach ($xml->xpath('//lk') as $lk) {
    $lks .= $lk . '; ';
  }
  $form['lk'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($lks),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Links - lk')
  );
  $dos = '';
  foreach ($xml->xpath('//do') as $do) {
    $dos .= $do . ' ';
  }
  $form['do'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($dos),
    '#title' => t('Document Object Identifier - do')
  );
  $cns = '';
  foreach ($xml->xpath('//cn') as $cn) {
    $cns .= $cn . ' ';
  }
  $form['cn'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($cns),
    '#title' => t('Call Number - cn')
  );
  $dbs = '';
  foreach ($xml->xpath('//db') as $db) {
    $dbs .= $db . ' ';
  }
  $form['db'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($dbs),
    '#title' => t('Database - db')
  );
  $dss = '';
  foreach ($xml->xpath('//ds') as $ds) {
    $dss .= $ds . ' ';
  }
  $form['ds'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($dss),
    '#title' => t('Accession Number - an')
  );
  $ips = '';
  foreach ($xml->xpath('//ip') as $ip) {
    $ips .= $ip . ' ';
  }
  $form['ip'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($ips),
    '#title' => t('Identifying Phrase - ip')
  );
  $rds = '';
  foreach ($xml->xpath('//rd') as $rd) {
    $rds .= $rd . ' ';
  }
  $form['rd'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($rds),
    '#title' => t('Retrieved Data - rd')
  );
  $sts = '';
  foreach ($xml->xpath('//st') as $st) {
    $sts .= $st . ' ';
  }
  $form['st'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($sts),
    '#title' => t('Shortened Title - st')
  );
  $u1s = '';
  foreach ($xml->xpath('//u1') as $u1) {
    $u1s .= $u1 . '; ';
  }
  $form['u1'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($u1s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('UPEI Username - u1')
  );
  $u2s = '';
  foreach ($xml->xpath('//u2') as $u2) {
    $u2s .= $u2 . '; ';
  }
  $form['u2'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($u2s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('UPEI Departments - u2')
  );
  $u3s = '';
  foreach ($xml->xpath('//u3') as $u3) {
    $u3s .= $u3 . '; ';
  }
  $form['u3'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($u3s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Unused - u3')
  );
  $u4s = '';
  foreach ($xml->xpath('//u4') as $u4) {
    $u4s .= $u4 . '; ';
  }
  $form['u4'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($u4s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Unused - u4')
  );
  $u5s = '';
  foreach ($xml->xpath('//u5') as $u5) {
    $u5s .= $u5 . '; ';
  }
  $form['u5'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($u5s),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('Unused - u5')
  );
  $uls = '';
  foreach ($xml->xpath('//ul') as $ul) {
    $uls .= $ul . '; ';
  }
  $form['ul'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($uls),
    '#description' => t('Separate multiple entries with a semicolon'),
    '#title' => t('URL - ul')
  );
  $sls = '';
  foreach ($xml->xpath('//sl') as $sl) {
    $sls .= $sl . ' ';
  }
  $form['sl'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($sls),
    '#title' => t('Sponsoring Library - sl')
  );
  $lls = '';
  foreach ($xml->xpath('//ll') as $ll) {
    $lls .= $ll . ' ';
  }
  $form['ll'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($lls),
    '#title' => t('Sponsoring Library Location - ll')
  );
  $crs = '';
  foreach ($xml->xpath('//cr') as $cr) {
    $crs .= $cr . ' ';
  }
  $form['cr'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($crs),
    '#title' => t('Cited Reference - cr')
  );
  $wts = '';
  foreach ($xml->xpath('//wt') as $wt) {
    $wts .= $wt . ' ';
  }
  $form['wt'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($wts),
    '#description' => 'This field does not display.  Put this info in the Primary Title Field.',
    '#title' => t('Website Title - wt')
  );
  $a6s = '';
  foreach ($xml->xpath('//a6') as $a6) {
    $a6s .= $a6 . '; ';
  }
  $form['a6'] = array(
    '#type' => 'textarea',
    '#default_value' => html_entity_decode($a6s),
    '#description' => t('This field does not display.  Put this info in the Authos or Secondary Authers'),
    '#title' => t('Website Editor - a6')
  );
  $wvs = '';
  foreach ($xml->xpath('//wv') as $wv) {
    $wvs .= $wv . ' ';
  }
  $form['wv'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($wvs),
    '#title' => t('Website Version - wv')
  );
  $wps = '';
  foreach ($xml->xpath('//wp') as $wp) {
    $wps .= $wp . ' ';
  }
  $form['wp'] = array(
    '#type' => 'textfield',
    '#default_value' => html_entity_decode($wps),
    '#title' => t('Date of Electronic Publication - wp')
  );
  return $form;
}

/**
 * Submit hook for the refworks metadata form.
 * 
 * @param array $form
 * @param array $form_state 
 */
function scholar_refworks_form_submit(array $form, array &$form_state) {
  $refworks = scholar_refworks_create_refworks_document($form_state);
  if (!scholar_refworks_update_refworks_datastream($refworks, $form_state)) {
    drupal_set_message('Failed to update Refworks datastream.');
  }
  $dublin_core = scholar_refworks_create_dublin_core_document($refworks);
  if (!scholar_refworks_update_dublin_core_datastream($dublin_core, $form_state)) {
    drupal_set_message('Failed to update Dublin Core datastream.');
  }
}

/**
 * Creates a Refworks metadata record from the submitted form values.
 * 
 * @param array $form_values 
 * 
 * @return DOMDocument
 */
function scholar_refworks_create_refworks_document(array &$form_state) {
  $form_values = $form_state['values'];
  $refworks = new DomDocument("1.0", "UTF-8");
  $refworks->substituteEntities = FALSE;
  $refworks->formatOutput = TRUE;
  $reference = $refworks->createElement("reference");
  $refworks->appendChild($reference);
  foreach ($form_values as $key => $value) {
    $value = htmlspecialchars(trim($value), ENT_NOQUOTES, 'UTF-8');
    if ($key != 'form_token' && $key != 'form_id' && $key != 'submit' && $key != 'pid' && $key != 'op' && $value != '') {
      if ($key == 'a1' || $key == 'u1' || $key == 'u2' || $key == 'a2' || $key == 'a3' || $key == 'a4' ||
          $key == 'a5' || $key == 'a6' || $key == 'lk' || $key == 'k1' || $key == 'ul') {
        $elements = explode(';', $value);
        foreach ($elements as $element) {
          if ($element != '') {
            $reference->appendChild($refworks->createElement($key, htmlentities($element)));
          }
        }
      }
      else {
        $reference->appendChild($refworks->createElement($key, $value));
      }
    }
  }
  return $refworks;
}

/**
 * Updates the Refworks metadata record.
 * 
 * @param DOMDocument $refworks
 * @param array $form_state
 * 
 * @return boolean
 *   TRUE if successful FALSE otherwise.
 */
function scholar_refworks_update_refworks_datastream(DOMDocument $refworks, array &$form_state) {
  module_load_include('inc', 'Fedora_Repository', 'ObjectHelper');
  $object = new ObjectHelper();
  $params = array(
    "pid" => $form_state['values']['pid'],
    "dsID" => 'refworks',
    "altIDs" => "",
    "dsLabel" => "refworks record",
    "MIMEType" => "text/xml",
    "formatURI" => "URL",
    "dsContent" => $refworks->saveXML(), "checksumType" => "DISABLED", "checksum" => "none", "logMessage" => "refworks_datastream_modified", "force" => "true");
  return $object->modifyDatastreamByValue($params); // Only works if its inline-xml
}

/**
 * Creates the Dublin Core metadata record.
 * 
 * @param DOMDocument $refworks 
 * 
 * @return DOMDocument
 */
function scholar_refworks_create_dublin_core_document(DOMDocument $refworks) {
  $dc = new DomDocument("1.0", "UTF-8");
  $oai_dc = $dc->createElement("oai_dc:dc");
  $oai_dc->setAttribute('xmlns:oai_dc', "http://www.openarchives.org/OAI/2.0/oai_dc/");
  $oai_dc->setAttribute('xmlns:dc', "http://purl.org/dc/elements/1.1/");
  $oai_dc->setAttribute('xmlns:dcterms', "http://purl.org/dc/terms/");
  $oai_dc->setAttribute('xmlns:xsi', "http://www.w3.org/2001/XMLSchema-instance");
  $dc->appendChild($oai_dc);
  $reference = simplexml_import_dom($refworks);
  foreach ($reference->sr as $value) {
    $element = $dc->createElement('dc:type', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->id as $value) {
    $identifier = $value;
  }
  foreach ($reference->a1 as $value) {
    $element = $dc->createElement('dc:creator', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->t1 as $value) {
    $form_values['dc:title'] = $value;
    $element = $dc->createElement('dc:title', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->jf as $value) {
    $source = (string) $value;
  }
  foreach ($reference->yr as $value) {
    $element = $dc->createElement('dc:date', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->vo as $value) {
    $source .= ' Volume: ' . (string) $value;
  }
  foreach ($reference->is as $value) {
    $source .= ' Issue: ' . (string) $value;
  }
  foreach ($reference->sp as $value) {
    $source .= ' Start Page: ' . (string) $value;
  }
  foreach ($reference->op as $value) {
    $source .= ' Other Pages: ' . (string) $value;
  }
  foreach ($reference->ul as $value) {
    $source .= ' URL: ' . (string) $value;
  }
  foreach ($reference->k1 as $value) {
    $element = $dc->createElement('dc:subject', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->a2 as $value) {
    $element = $dc->createElement('dc:contributor', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->a3 as $value) {
    $element = $dc->createElement('dc:contributor', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->a4 as $value) {
    $element = $dc->createElement('dc:contributor', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->a5 as $value) {
    $element = $dc->createElement('dc:contributor', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->la as $value) {
    $element = $dc->createElement('dc:language', htmlspecialchars((string) $value, ENT_NOQUOTES, 'UTF-8'));
    $oai_dc->appendChild($element);
  }
  foreach ($reference->pb as $value) {
    $source = 'Publisher: ' . (string) $value;
  }
  foreach ($reference->pp as $value) {
    $source .= ' Place of Publication: ' . (string) $value;
  }
  foreach ($reference->sn as $value) {
    $identifier .= ' ISSN/ISBN: ' . (string) $value;
    $this->romeoUrlString = $this->romeoUrlString . (string) $value;
  }
  foreach ($reference->ab as $value) {
    $description = ' abstract: ' . (string) $value;
  }
  foreach ($reference->cr as $value) {
    $description .= ' Cited Reference: ' . (string) $value;
  }
  $element = $dc->createElement('dc:description', htmlspecialchars($description, ENT_NOQUOTES, 'UTF-8'));
  $oai_dc->appendChild($element);
  $element = $dc->createElement('dc:source', htmlspecialchars($source, ENT_NOQUOTES, 'UTF-8'));
  $oai_dc->appendChild($element);
  $element = $dc->createElement('dc:identifier', htmlspecialchars($idenitifier, ENT_NOQUOTES, 'UTF-8'));
  $oai_dc->appendChild($element);
  return $dc;
}

/**
 * Updates the Dublin Core metadata record.
 * 
 * @param DOMDocument $dublin_core
 * @param array $form_state
 * 
 * @return boolean
 *   TRUE if successful FALSE otherwise.
 */
function scholar_refworks_update_dublin_core_datastream(DOMDocument $dublin_core, array &$form_state) {
  module_load_include('inc', 'Fedora_Repository', 'ObjectHelper');
  $object = new ObjectHelper();
  $params = array(
    "pid" => $form_state['values']['pid'],
    "dsID" => 'DC',
    "altIDs" => "",
    "dsLabel" => "Default Dublin Core Record",
    "MIMEType" => "text/xml",
    "formatURI" => "URL",
    "dsContent" => $dublin_core->saveXML(), "checksumType" => "DISABLED", "checksum" => "none", "logMessage" => "dc_datastream_modified", "force" => "true");
  return $object->modifyDatastreamByValue($params); // Only works if its inline-xml
}