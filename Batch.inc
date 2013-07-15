<?php

/**
 * @file
 * 
 * Collection of functions related to Batch ingests.
 */

/**
 * Ingests multiple objects one for each record within the given EndNote XML document.
 * 
 * @param string $filename
 *   The name of the EndNote XML file to ingest.
 * @param type $collection_pid
 *   The pid of the collection to ingest into.
 * @param type $content_model_pid
 *   The content model to assign to the new pid.
 * @param array $context 
 *   The drupal batch context.
 */
function scholar_batch_ingest_endnote_document($filename, $collection_pid, $content_model_pid, array &$context) {
  module_load_include('inc', 'bibutils', 'Batch');
  module_load_include('inc', 'bibutils', 'Bibutils');
  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = bibutils_batch_get_number_endnote_records($filename);
  }
  $temp_file_dir = file_directory_path(); // Changed the temp dir so that the files can be accessed
  //these filenames should be generated somehow as right now if multiple users
  //tried to do a batch ingest they would be stepping on each other.
  $mods_filename = file_create_filename('mods.xml', $temp_file_dir);
  $endnote_filename = file_create_filename('endnote.xml', $temp_file_dir);
  $record = bibutils_batch_get_endnote_record($filename); // Get Single EndNote XML Record
  $record->save($endnote_filename);
  if (Bibutils::Convert($endnote_filename, 'EndNoteXML', $mods_filename, 'MODS')) {
    Bibutils::CleanModsFile($mods_filename);
    $endnote_url = file_directory_path() . '/endnote.xml';
    scholar_batch_ingest_mods($mods_filename, $collection_pid, $content_model_pid, array('ENDNOTE' => $endnote_url));
  }
  unlink($mods_filename);
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
}

/**
 * takes a refworks xml file with multiple reference elements and creates 
 * a new fedora object for each reference.
 * 
 * @param string $filename
 * @param string $collection_pid
 * @param string $content_model_pid
 * @param array $context 
 */
function scholar_batch_ingest_refworks_document($filename, $collection_pid, $content_model_pid, array &$context) {
  $doc = new DOMDocument();
  $xml = file_get_contents($filename);
  static $file_list = array();
  $xml = trim($xml); //refworks has a nasty habit of outputing an xml file with 
  // the xml declartion being the second line, the first line being blank
  $success = $doc->loadXML($xml);
  $root = $doc->documentElement;
  $nodeName = $root->nodeName;
  if (!$success || !$nodeName == 'refworks') {
    drupal_set_message(t('error doing refworks batch ingest'));
    return;
  }
  $references = $doc->getElementsByTagName('reference');
  $count = $references->length;

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = $count;
  }

  $reference = $references->item($context['sandbox']['progress']);
  $title = $reference->getElementsByTagName('t1');
  $title_text = $title->item(0)->nodeValue;
  $context['message'] = t('Added %title', array('%title' => $title_text));
  $rxml = $doc->saveXML($reference);
  $mods = scholar_batch_ingest_mods_from_refworks($reference, $rxml);
  $refworks_filename = file_create_filename(scholar_batch_create_filename('Refworks_', '.xml')
      , file_directory_path());
  $mods_filename = file_create_filename(scholar_batch_create_filename('sm_', '.xml')
      , file_directory_path());
  $policy_filename = file_create_filename(scholar_batch_create_filename('POLICY_', '.xml')
      , file_directory_path());
  module_load_include('inc', 'islandora_xacml_api', 'Xacml');
  $xacml = new Xacml();
  scholar_batch_add_xacml_users_and_roles($reference, $xacml);
  $xacml_string = $xacml->getXmlString();
  file_put_contents($policy_filename, $xacml_string);
  file_put_contents($mods_filename, $mods);
  file_put_contents($refworks_filename, $rxml);
  scholar_batch_ingest_mods($mods_filename, $collection_pid, $content_model_pid
      , array('refworks' => $refworks_filename,
    'POLICY' => $policy_filename));
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total']) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
  else {
    unlink($filename);
  }
}

/**
 * adds the appropriate users and roles to an xacml policy
 * 
 * @param DOMElement $reference
 * @param XACML object $xacml 
 */
function scholar_batch_add_xacml_users_and_roles($reference, &$xacml) {
  $users = $reference->getElementsByTagName('u1');
  $roles = $reference->getElementsByTagName('u2');
  foreach ($users as $user) {
    $us = explode(';', $user->nodeValue);
    foreach ($us as $u) {
      $xacml->managementRule->addUser($u);
    }
  }
  foreach ($roles as $role) {
    $rs = explode(';', $role->nodeValue);
    foreach ($rs as $r) {
      $xacml->managementRule->addRole($r);
    }
  }
}

/**
 *
 * @global object $user
 *  drupal user object
 * @param string $prefix
 *   filename prefix
 * @param string $suffix
 *   filename suffix
 * @return string 
 *   a unique filename
 */
function scholar_batch_create_filename($prefix = "", $suffix = "") {
  global $user;
  return $prefix . md5(uniqid($user->uid . rand(), TRUE)) . $suffix;
}

/**
 *
 * @param DOMElement $reference
 * @param string $rxml
 *   refworks xml
 * @return xml
 *   a mods xml file 
 */
function scholar_batch_ingest_mods_from_refworks($reference, $rxml) {
  $genre = $reference->getElementsByTagName('rt');
  $genre_text = $genre->item(0)->nodeValue;
  $path = drupal_get_path('module', 'scholar');
  $xslt_path = NULL;
  switch ($genre_text) {
    case 'Book, Section':
      $xslt_path = $path . '/xsl/refworks_to_mods_book_section.xsl';
      break;
    case 'Book, Whole':
    case 'Sound Recording':
    case 'Dissertation/Thesis':
    case 'Dissertation/Thesis, Unpublished':
      $xslt_path = $path . '/xsl/refworks_to_mods_book.xsl';
      break;
    case 'Conference Proceedings':
      $xslt_path = $path . '/xsl/refworks_to_mods_conf.xsl';
      break;
    default:
      $xslt_path = $path . '/xsl/refworks_to_mods_journal.xsl';
  }
  $rdoc = new DOMDocument();
  $xslt = new XSLTProcessor();
  $rdoc->loadXML($rxml);
  $xslt_doc = new DOMDocument();
  $xslt_doc->load($xslt_path);
  $xslt->importStylesheet($xslt_doc);
  $mods = $xslt->transformToXml($rdoc);
  return $mods;
}

/**
 * Batch ingest the RIS document.
 * 
 * @param string $filename
 *   The name of the EndNote XML file to ingest.
 * @param type $collection_pid
 *   The pid of the collection to ingest into.
 * @param type $content_model_pid
 *   The content model to assign to the new pid.
 * @param array $context 
 *   The drupal batch context.
 */
function scholar_batch_ingest_ris_document($filename, $collection_pid, $content_model_pid, array &$context) {
  module_load_include('inc', 'bibutils', 'Batch');
  module_load_include('inc', 'bibutils', 'Bibutils');
// This code removes ^M characters that are present in exported Refworks files
// which don't show up as new lines in linux.
  $initial_ris = file_get_contents($filename);
  $cleaned_ris = str_replace("\x0D", "\n", trim($initial_ris));
  $initial_file = fopen($filename, 'w');
  fwrite($initial_file, $cleaned_ris);
  fclose($initial_file);

  if (empty($context['sandbox'])) {
    $context['sandbox']['progress'] = 0;
    $context['sandbox']['total'] = bibutils_batch_get_number_ris_records($filename);
  }

  $temp_file_dir = file_directory_path(); // Changed the temp dir so that the files can be accessed
  $mods_filename = file_create_filename('mods.xml', $temp_file_dir);
  $ris_filename = file_create_filename('ris.txt', $temp_file_dir);
  $record = bibutils_batch_get_ris_record($filename); // Get Single RIS Record
  $file = fopen($ris_filename, 'w');
  fwrite($file, $record);
  fclose($file);

  if (Bibutils::Convert($ris_filename, 'RIS', $mods_filename, 'MODS')) {
    $ris_url = file_directory_path() . '/ris.txt';
    scholar_batch_ingest_mods($mods_filename, $collection_pid, $content_model_pid, array('RIS' => $ris_url));
  }
// Remove the files.
  unlink($ris_filename);
  unlink($mods_filename);
  $context['sandbox']['progress']++;
  if ($context['sandbox']['progress'] != $context['sandbox']['total'] && $context['sandbox']['total'] != 0) {
    $context['finished'] = $context['sandbox']['progress'] / $context['sandbox']['total'];
  }
}

/**
 * todo add comments
 * 
 * @param string $filename
 * @param string $collection_pid
 * @param string $content_model_pid
 * @param array $datastreams 
 */
function scholar_batch_ingest_mods($filename, $collection_pid, $content_model_pid, $datastreams) {
  module_load_include('inc', 'islandora_content_model_forms', 'FOXML');
  module_load_include('inc', 'fedora_repository', 'CollectionPolicy');
  $collection_policy = CollectionPolicy::loadFromCollection($collection_pid);
  if ($collection_policy !== FALSE) {
    $document = new DOMDocument();
    $document->load($filename);
    $label = scholar_batch_ingest_mods_guess_title($document); // Find a best guess solution... to pull from the mods.
    $pid = $collection_policy->getNextPid('ISLANDORACM');
    $dsid = 'MODS';
    $relationship = $collection_policy->getRelationship();
    $ingest_file_location = $datastreams;
    $transform = drupal_get_path('module', 'scholar') . '/xsl/mods_to_dc.xsl';
    $state = 'A';
    $foxml = new FOXML($label, $pid, $dsid, $content_model_pid, $collection_pid, $relationship, $ingest_file_location, $document, $transform, $state);
    $foxml->ingest();
    unlink($filename); //the batch ingest functions removes the policy and refworks files but not the mods
  }
}

/**
 * Guess the title for the object generated from mods.
 * 
 * @param DOMDocument $mods
 *   The MODS document in which to extract the title from.
 * 
 * @return string
 *   The extracted title or a default title.
 */
function scholar_batch_ingest_mods_guess_title(DOMDocument $mods) {
  $default_title = t('Default Title (Please Correct)');
  $xpath = new DOMXPath($mods);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');
  $results = $xpath->query('/mods:mods/mods:titleInfo[not(@type)]/mods:title[1]'); // Ingnore extras.
  if ($results->length == 1) {
    $title = $results->item(0);
    return $title->textContent;
  }
  $results = $xpath->query('/mods:mods/mods:titleInfo/mods:title[1]'); // Ingnore extras, can be abbr or whatever.
  if ($results->length > 0) { // Can be more than one.
    $title = $results->item(0); // Get first
    return $title->textContent;
  }
  return $default_title;
}

function islandora_scholar_get_pmid_data($id) {

  $pubmed_url = 'http://www.ncbi.nlm.nih.gov/entrez/eutils/efetch.fcgi';
  $url = url($pubmed_url, array(
    'query' => array(
      'db' => 'pubmed',
      'retmode' => 'xml',
      'id' => $id,
      )));
  $response = drupal_http_request($url);
  if ($response->data) {
    $pubmed_xml = simplexml_load_string($response->data);
    if ($id === getPMID($pubmed_xml)) {
      return $response->data;
    }
    else {
      return FALSE;
    }
  }
}

function getPMID(SimpleXMLElement $xml) {
  return (string) array_shift($xml->xpath('//PubmedArticle/MedlineCitation/PMID'));
}

function islandora_scholar_build_pubmed_object($id, &$context) {
  global $user;
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
  }
  $context['sandbox']['progress'] = $context['sandbox']['progress'] + 1;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pmed_data = islandora_scholar_get_pmid_data($id);
   if ($pmed_data === FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $mods = islandora_scholar_get_mods($pmed_data);
   if ($mods == FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $dc = islandora_scholar_get_dc($mods);
  $xml = simplexml_load_string($dc);
  if ($xml === FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $dcData = $xml->children();
  if (is_array($dcData->title)) {
    $label = $dcData->title[0];
  }
  else {
    $label = $dcData->title;
  }
  $collection_pid = variable_get('islandora_scholar_new_citation_collection', 'ir:collection');
  $pid = fedora_item::get_next_PID_in_namespace('ir');
  $owner = ($user->uid > 0) ? $user->name : 'anonymous';
  $item = Fedora_item::ingest_new_item($pid, 'A', $label, $owner);
  $item->add_relationship('hasModel', 'islandora:citationCModel', FEDORA_MODEL_URI);
  $item->add_relationship('isMemberOfCollection', $collection_pid);
  // Uncomment line below if you wish to persist original PMED data
  //$item->add_datastream_from_string($pmed_data, 'PMED');
  $item->add_datastream_from_string($mods, 'MODS');
  $item->modify_datastream_by_value($dc, 'DC', "Dublin Core", 'text/xml', 'X');
  $context['results']['success'][] = t('successfully created %id = %pid', array('%id' => $id, '%pid' => $pid));
}

function islandora_scholar_build_pubmed_object_finished($success, $results, $operations) {
  if (isset($results['failed'])) {
    $error_message .= theme('item_list', $results['failed']);
    drupal_set_message($error_message, 'error');
  }
  if (isset($results['success'])) {
    $message .= theme('item_list', $results['success']);
    drupal_set_message($message);
  }
}

function islandora_scholar_build_doi_object_finished($success, $results, $operations) {
  islandora_scholar_build_pubmed_object_finished($success, $results, $operations);
}

function islandora_scholar_get_mods($pmed_data) {
  $xsl = new DOMDocument();
  $xsl->load(drupal_get_path('module', 'scholar') . '/xsl/pubmed_to_mods.xslt');
  $pm_xml = DOMDocument::loadXML($pmed_data);
  $xslt = new XSLTProcessor();
  $xslt->importStylesheet($xsl);
  $pm_xml = $xslt->transformToDoc($pm_xml->documentElement);
  $pm_xml = islandora_scholar_correctTransform($pm_xml);
  $mods = $pm_xml->saveXML();
  return $mods;
}

function islandora_scholar_get_dc($mods) {
  $xsl = new DOMDocument();
  $xsl->load(drupal_get_path('module', 'scholar') . '/xsl/mods_to_dc.xsl');
  $mods_xml = DOMDocument::loadXML($mods);
  $xslt = new XSLTProcessor();
  $xslt->importStylesheet($xsl);
  $dc_xml = $xslt->transformToDoc($mods_xml->documentElement);
  $dc = $dc_xml->saveXML();
  return $dc;
}

/**
 * Normalize some fields.
 */
function islandora_scholar_correctTransform(DOMDocument $doc) {
  $xpath = new DOMXPath($doc);
  $xpath->registerNamespace('mods', 'http://www.loc.gov/mods/v3');

  // Correct dateIssued/dateCaptured
  $results = $xpath->query('//mods:dateIssued | //mods:dateCaptured');
  $count = $results->length;
  for ($i = 0; $i < $count; $i++) {
    $date = $results->item($i);
    normalizeDate($date, $xpath);
  }

  // Add the record creation date.
  $results = $xpath->query('//mods:recordInfo/mods:recordCreationDate');
  $count = $results->length;
  for ($i = 0; $i < $count; $i++) {
    $date = $results->item($i);
    $date_time = new DateTime();
    $date->nodeValue = $date_time->format(DateTime::ISO8601) . 'Z';
  }

  return $xpath->document;
}

/**
 * Normalizes dates used in PubMed, only supports PubDate and ArticleDate.
 *
 * Torn from CoH.
 * Note: Can't normalize <MedlineDate> values.
 *
 * @param DOMElement $date
 *   The element containing the PubMed date.
 * @param DOMXPath $xpath
 *   The xpath used to fetch the date.
 */
function normalizeDate(DOMElement $date, DOMXPath $xpath) {
  $months = array(
    'Jan', 'Feb', 'Mar', 'Apr', 'May', 'Jun', 'Jul', 'Aug', 'Sep', 'Oct',
    'Nov', 'Dec');
  $iso = $xpath->evaluate('count(mods:MedlineDate) = 0', $date);
  if ($iso) {
    $date->setAttribute('encoding', 'iso8601');
    $y = $xpath->evaluate('normalize-space(mods:Year/text())', $date);
    $m = $xpath->evaluate('normalize-space(mods:Month/text())', $date);
    $d = $xpath->evaluate('normalize-space(mods:Day/text())', $date);
  }
  else {
    $string = $xpath->evaluate('normalize-space(mods:MedlineDate/text())', $date);
    if (preg_match('/^[0-9]{4}-[0-9]{2}-[0-9]{2}$/', $string)) {
      list($y, $m, $d) = explode('-', $string);
    }
    elseif (preg_match('/^[0-9]{4}-[0-9]{2}$/', $string)) {
      list($y, $m) = explode('-', $string);
    }
    elseif (preg_match('/^[0-9]{4}$/', $string)) {
      $y = $string;
    }
    elseif (preg_match('/^[0-9]{4}-[0-9]{4}$/', $string)) {
      $y = substr($string, 0, 4);
    }
    elseif (preg_match('/^[0-9]{4} [A-Za-z]{3}-[A-Za-z]{3}$/', $string)) {
      $y = substr($string, 0, 4);
      $m = substr($string, 5, 3);
    }
    elseif (preg_match('/^[0-9]{4} [A-Za-z]{3}$/', $string)) {
      $y = substr($string, 0, 4);
      $m = substr($string, 5, 3);
    }
    elseif (preg_match('/^[0-9]{4}-[A-Za-z]{3}$/', $string)) {
      $y = substr($string, 0, 4);
      $m = substr($string, 5, 3);
    }
  }
  $key = array_search(strtolower($m), array_map('strtolower', $months));
  if ($key !== FALSE) {
    $m = $key + 1;
  }
  $values = array(&$y, &$m, &$d);
  foreach ($values as &$val) {
    // Pad mth and day with 0.
    $val = (strlen($val) < 2 && $val) ? str_pad($val, 2, "0", STR_PAD_LEFT) : $val;
  }
  if ($y) {
    $string = $y;
    if ($m) {
      $string .= '-' . $m;
      if ($d) {
        $string .= '-' . $d;
      }
    }
  }
  $date->nodeValue = $string;
}

function islandora_scholar_get_doi_data($id) {
  $openurl = variable_get('doi_importer_openurl', 'http://www.crossref.org/openurl');
  $openurl_pid = variable_get('doi_importer_openurl_pid', 'zac@yesguy.biz');
  $url = url($openurl, array(
    'query' => array(
      'noredirect' => 'true',
      'pid' => $openurl_pid,
      'format' => 'unixref',
      'id' => ((strpos($id, 'doi:') === 0) ? $id : 'doi:' . $id),
      )));
  $response = drupal_http_request($url);
  $headers = array_change_key_case($response->headers);
  if ($response->data &&
      strpos($headers['content-type'], 'text/html') === FALSE &&
      strpos($response->data, "Malformed DOI") === FALSE) {

    $crossref_xml = simplexml_load_string($response->data);
    if ($crossref_xml->doi_record->crossref->error) {
      return FALSE;
    }
    else {
      return $response->data;
    }
  }
  else {
    return FALSE;
  }
}

function islandora_scholar_build_doi_object($id, &$context) {
  global $user;
  if (!isset($context['sandbox']['progress'])) {
    $context['sandbox']['progress'] = 0;
  }
  $context['sandbox']['progress'] = $context['sandbox']['progress'] + 1;
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $doi_data = islandora_scholar_get_doi_data($id);
  if ($doi_data === FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $mods = islandora_repository_get_MODS_from_doi($doi_data);
  $dc = islandora_scholar_get_dc($mods);
  if ($dc === FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $xml = simplexml_load_string($dc);
  $xml = simplexml_load_string($dc);
  if ($xml === FALSE) {
    $context['results']['failed'][] = t('failed creating object for %id ', array('%id' => $id));
    return FALSE;
  }
  $dcData = $xml->children('dc', TRUE);
  if (is_array($dcData->title)) {
    $label = $dcData->title[0];
  }
  else {
    $label = $dcData->title;
  }
  $collection_pid = variable_get('islandora_scholar_new_citation_collection', 'ir:collection');
  $pid = fedora_item::get_next_PID_in_namespace('ir');
  $owner = ($user->uid > 0) ? $user->name : 'anonymous';
  $item = Fedora_item::ingest_new_item($pid, 'A', $label, $owner);
  $item->add_relationship('hasModel', 'islandora:citationCModel', FEDORA_MODEL_URI);
  $item->add_relationship('isMemberOfCollection', $collection_pid);
  $item->add_datastream_from_string($mods, 'MODS');
  $item->modify_datastream_by_value($dc, 'DC', "Dublin Core", 'text/xml', 'X');
  $context['results']['success'][] = t('successfully created %id = %pid', array('%id' => $id, '%pid' => $pid));
}

function islandora_repository_get_MODS_from_doi($incoming_doi) {
  $mods = new SimpleXMLElement('<mods xmlns="http://www.loc.gov/mods/v3" xmlns:mods="http://www.loc.gov/mods/v3" xmlns:xlink="http://www.w3.org/1999/xlink"/>');
  // @todo Implement book support.
  $doi_xml = simplexml_load_string($incoming_doi);
  foreach ($doi_xml->doi_record as $record) {
    foreach ($record->crossref as $crossref) {
      foreach ($crossref->journal as $journal) {
        foreach ($journal->journal_metadata as $metadata) {
          $relateditem = $mods->addChild('relatedItem');
          $relateditem->addAttribute('type', 'host');
          foreach ($metadata->attributes() as $name => $item) {
            switch ((string) $name) {
              case 'language':
                $language = (string) $item[0];
            }
          }
          foreach ($metadata->children() as $name => $item) {
            switch ((string) $name) {
              case 'full_title':
                $titleinfo = $relateditem->addChild('titleInfo');
                $titleinfo->addChild('title', (string) $item);
                break;

              case 'abbrev_title':
                $titleinfo = $relateditem->addChild('titleInfo');
                $titleinfo->addAttribute('type', 'abbreviated');
                $titleinfo->addChild('title', (string) $item);
                break;

              case 'issn':
                $issn = $relateditem->addChild('identifier', (string) $item);
                $issn->addAttribute('type', 'issn');
                break;

              case 'coden':
                break;

              case 'doi_data':
                break;
            }
          }
        }
        foreach ($journal->journal_issue as $issue) {
          foreach ($issue->children() as $name => $item) {
            switch ((string) $name) {
              case 'contributors':
                break;

              case 'publication_date':
                $pubdate_year = (string) $item[0]->year[0];
                $pubdate_month = (string) $item[0]->month[0];
                $pubdate_day = (string) $item[0]->day[0];
                break;

              case 'journal_volume':
                $volume_number = (string) $item[0]->volume[0];
                break;

              case 'issue':
                $issue_number = (string) $item[0];
                break;

              case 'special_numbering':
                break;

              case 'doi_data':
                break;
            }
          }
        }
        foreach ($journal->journal_article as $article) {
          //we know it's a journal article
          $mods->addChild('genre', 'Journal Article');
          foreach ($article->attributes() as $name => $item) {
            switch ((string) $name) {
              case 'publication_type':
                // Options: full_text, abstract_only, bibliographic_record
                // doesn't seem to matter here.
                break;

              case 'language':
                $language = $mods->addChild('language');
                $languageterm = $language->addChild('languageTerm', (string) $item[0]);
                if (strlen((string) $item[0]) == 2) {
                  $languageterm->addAttribute('type', 'code');
                  $languageterm->addAttribute('authority', 'rfc4646');
                }
                break;
            }
          }
          foreach ($article->children() as $name => $item) {
            switch ((string) $name) {
              case 'titles':
                $title_info = $mods->addChild('titleInfo');
                $title_info->addChild('title', (string) $item->title[0]);
                if ($item->subtitle) {
                  $title_info->addChild('subTitle', (string) $item->subtitle[0]);
                }
                if ($item->original_language_title) {
                  // XXX: doesn't seem to actually add the original language
                  // title... creates the element, with attributes, but no
                  // content?
                  $title_info_translated = $mods->addChild('titleInfo');
                  $title_info_translated->addAttribute('type', 'translated');
                  $title_info_translated->addAttribute('lang', (string) $item->original_language_title['language']);
                }
                break;

              case 'contributors':
                foreach ($item->children() as $type => $contributor) {
                  switch ((string) $type) {
                    case 'person_name':
                      $role = strtolower((string) $contributor['contributor_role']);
                      $sequence = (string) $contributor['sequence'];
                      $given_name = (string) $contributor->given_name;
                      $surname = (string) $contributor->surname;
                      $suffix = (string) $contributor->suffix;
                      $affiliation = (string) $contributor->affiliation;

                      $mods_name = $mods->addChild('name');
                      $mods_name->addAttribute('type', 'personal');

                      $given_name_part = $mods_name->addChild('namePart', $given_name);
                      $given_name_part->addAttribute('type', 'given');
                      $surname_part = $mods_name->addChild('namePart', $surname);
                      $surname_part->addAttribute('type', 'family');

                      if ($sequence == 'first') {
                        $mods_name->addAttribute('usage', 'primary');
                      }

                      if ($role) {
                        $name_role = $mods_name->addChild('role');
                        $name_role->addChild('roleTerm', $role);
                        $roles = array('author', 'editor', 'translator');
                        if (in_array($role, $roles)) {
                          $name_role->roleTerm->addAttribute('authority', 'marcrelator');
                          $name_role->roleTerm->addAttribute('type', 'text');
                        }
                      }

                      if ($affiliation) {
                        $mods_name->addChild('affiliation', $affiliation);
                      }

                      if ($suffix) {
                        $terms_of_address = $mods_name->addChild('namePart', $suffix);
                        $terms_of_address->addAttribute('type', 'termsOfAddress');
                      }

                      break;

                    case 'organization':
                      $role = strtolower((string) $contributor['contributor_role']);
                      $sequence = (string) $contributor['sequence'];
                      $mods_name = $mods->addChild('name');
                      $mods_name->addAttribute('type', 'corporate');
                      $mods_name->addChild('namePart', (string) $contributor);
                      break;
                  }
                }
                break;

              // genre element was not getting populated and it is needed for proper CSL display.
              // If we start pulling anything other than journal articles using DOIs, then this will need to be revisited. 
              // journal article will work when piped through CSL converter.
              case 'genre':
                $genre = 'journal article';
                $mods_genre = $mods->addChild('genre', $genre);
                break;

              case 'publication_date':
                $pubdate_year = (string) $item[0]->year[0];
                $pubdate_month = (string) $item[0]->month[0];
                $pubdate_day = (string) $item[0]->day[0];
                break;


              case 'pages':
                $first_page = (string) $item[0]->first_page[0];
                $last_page = (string) $item[0]->last_page[0];
                $other_pages = (string) $item[0]->other_pages[0];
                break;

              case 'publisher_item':
                // @todo.
                break;

              case 'crossmark':
                // @todo.
                break;

              case 'doi_data':
                foreach ($item->doi as $doi) {
                  $mods_identifier = $mods->addChild('identifier', (string) $doi);
                  $mods_identifier->addAttribute('type', 'doi');
                }
                foreach ($item->resource as $resource) {
                  $mods_identifier = $mods->addChild('identifier');
                  $mods_identifier->{0} = (string) $resource;
                  $mods_identifier->addAttribute('type', 'uri');
                }
                break;

              case 'citation_list':
                // @todo.
                break;

              case 'component_list':
                // @todo.
                break;
            }
          }
        }

        // we should populated the dateIssued element otherwise CSL does not display a date
        $originInfo = $mods->addChild('originInfo');
        if (!empty($pubdate_year)) {
          $date_value = $pubdate_year;
          if (!empty($pubdate_month)) {
            if (strlen($pubdate_month) == 1) {
              $pubdate_month = "0" . $pubdate_month;
            }
            $date_value .= "-" . $pubdate_month;
          }
          if (!empty($pubdate_day)) {
            if (strlen($pubdate_day) == 1) {
              $pubdate_day = "0" . $pubdate_day;
            }
            $date_value .= "-" . $pubdate_day;
          }
          $originInfo->addChild('dateIssued', $date_value);
        }



        if (!empty($volume_number) || !empty($issue_number) || !empty($other_pages) || !empty($last_page) || !empty($first_page) || !empty($pubdate_year) || !empty($pubdate_month) || !empty($pubdate_day)) {
          $part = $mods->addChild('part');

          if (!empty($last_page) || !empty($first_page) || !empty($other_pages)) {
            $extent = $part->addChild('extent');
            $extent->addAttribute('unit', 'page');
            if (!empty($other_pages)) {
              $extent->addChild('list', $other_pages);
            }
            if (!empty($last_page) || !empty($first_page)) {
              $extent->addChild('start', $first_page);
              $extent->addChild('end', $last_page);
            }
          }

          if (!empty($volume_number)) {
            $volume = $part->addChild('detail');
            $volume->addAttribute('type', 'volume');
            $volume->addChild('number', $volume_number);
          }
          if (!empty($issue_number)) {
            $issue = $part->addChild('detail');
            $issue->addAttribute('type', 'issue');
            $issue->addChild('number', $issue_number);
          }
          if (!empty($pubdate_year)) {
            $date_value = $pubdate_year;
            if (!empty($pubdate_month)) {
              if (strlen($pubdate_month) == 1) {
                $pubdate_month = "0" . $pubdate_month;
              }
              $date_value .= "-" . $pubdate_month;
            }
            if (!empty($pubdate_day)) {
              if (strlen($pubdate_day) == 1) {
                $pubdate_day = "0" . $pubdate_day;
              }
              $date_value .= "-" . $pubdate_day;
            }
            $part->addChild('date', $date_value);
          }
        }

        // Return after first instance.
        $mods = $mods->asXML();
      }
    }
  }


  return $mods;
}