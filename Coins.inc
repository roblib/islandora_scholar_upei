<?php

/**
 * @file
 * Coins.inc
 */

/**
 * Class to parse relevant data from MODS, initially used to 
 * construct the COINS url but has found use elsewhere.
 */
class Coins {

  public $pid = NULL;
  public $mods = NULL;
  public $issn = NULL;
  public $volume = NULL;
  public $issue = NULL;
  public $date = NULL;
  public $start_page = NULL;
  public $title = NULL;
  public $sub_title = NULL;
  public $journal_title = NULL;
  public $abstract = NULL;
  public $names = NULL;
  public $authors = NULL;
  public $contributors = NULL;
  public $reverse_names = NULL;
  public $topic = NULL;
  public $note = NULL;
  public $embargo_date = NULL;
  public $genre = NULL;
  public $family_names = NULL;
  public $full_names = NULL;
  public $username = NULL;
  public $department = NULL;
  public $type = NULL;

  /**
   * Create the object properties from the MODS datastream
   * 
   * @param type $pid 
   * The PID of the object that you want to extract the MODS data from
   * 
   * @return type 
   */
  function __construct($pid) {
    $this->pid = $pid;
    module_load_include('inc', 'Fedora_Repository', 'api/fedora_item');
    $object = new Fedora_Item($pid);
    $xmlstr = $object->get_datastream_dissemination('MODS');

    if ($xmlstr == NULL || strlen($xmlstr) < 5) {
      drupal_set_message(t('No MODS datastream found!', 'warning'));
      return " ";
    }
    $xml = new SimpleXMLElement($xmlstr);
    $xml->registerXPathNamespace('mods', 'http://www.loc.gov/mods/v3');

    $this->mods = $xml;

    $full_names = array();
    $names = array();
    $authors = array();
    $contributors = array();
    $family_names = array();
    $i = 0;
    foreach ($xml->name as $name) {
      foreach ($name->namePart as $name_part) {
        if ((string) $name_part != '') {
          if ($name_part['type'] == 'given') {
            $names[$i] .= (string) substr(rtrim($name_part, '.'), 0, 1) . '. ';
            $full_names[$i] .= (string) $name_part;
          }
          if ($name_part['type'] == 'family') {
            $names[$i] .= (string) rtrim($name_part, '.');
            $family_names[$i] = (string) rtrim($name_part, '.');
            $full_names[$i] .= ' ' . (string) rtrim($name_part, '.');
          }
        }
      }
      $role = (string) $name->role->roleTerm;
      if ($role == 'aut') {
        $role = 'author';
      }
      if ($role == 'author') {
        $authors[] = $names[$i];
      }
      else {
        $contributors[] = $names[$i];
      }
      $names[$i] .= ' (' . $role . ')';
      $i++;
    }
    $this->names = $names;
    $this->full_names = $full_names;
    $this->authors = $authors;
    $this->contributors = $contributors;
    $this->family_names = $family_names;

    $reverse = array();
    $i = 0;
    foreach ($xml->name as $name) {
      foreach ($name->namePart as $name_part) {
        if ($name_part['type'] == 'family') {
          $reverse_family[$i] .= (string) rtrim($name_part, '.');
        }
        if ($name_part['type'] == 'given') {
          $reverse_given[$i] .= (string) rtrim($name_part, '.');
        }
        $reverse[$i] = $reverse_family[$i] . ', ' . $reverse_given[$i] . '.';
      }
      $i++;
    }
    $this->reverse_names = $reverse;

    $issn = $xml->xpath('//mods:identifier[@type="issn"]');
    $this->issn = str_replace(array("&", "=", ",", ";"), array('', '', '', ''), (string) $issn[0]);

    $this->date = $xml->part->date;
    if ($this->date == NULL) {
      $this->date = $xml->originInfo->dateCreated;
    }

    $volume = $xml->xpath('//mods:part/mods:detail[@type="volume"]');
    $this->volume = $volume[0]->number;

    $issue = $xml->xpath('//mods:part/mods:detail[@type="issue"]');
    $this->issue = $issue[0]->number;

    $start_page = $xml->xpath('//mods:extent[@unit="page"]/mods:start');
    $this->start_page = $start_page[0];

    $title = $xml->xpath('//mods:title');
    $this->title = $title[0];

    $sub_title = $xml->xpath('//mods:title');
    $this->sub_title = $sub_title[0];

    $journal_title = $xml->xpath('//mods:relatedItem[@type="host"]//mods:title');
    $this->journal_title = $journal_title[0];

    $abstract = $xml->xpath('//mods:abstract');
    $this->abstract = $abstract[0];

    $topic = array();
    foreach ($xml->subject as $subject) {
      $topic[] = $subject->topic;
    }
    $this->topic = $topic;

    $notes = array();
    foreach ($xml->note as $note) {
      $notes[] = $note;
    }
    $this->note = $note;

    $embargo_date = $xml->xpath('//mods:dateOther[@type="embargo"]');
    $this->embargo_date = $embargo_date[0];

    $genre = $xml->xpath('//mods:genre');
    $this->genre = $genre;

    $username = $xml->xpath('//mods:identifier[@type="u1"]');
    $this->username = $username;

    $department = $xml->xpath('//mods:identifier[@type="u2"]');
    $this->department = $department;
  }

  /**
   * Function to return citation details in a table-ready form
   * 
   * @param string $type
   * Type of reference that is being used. Default value is citation but 
   * thesis and book are also valid. This parameter will change the values
   * that are returned when the render_view function is called
   * 
   * @return type 
   * An array containing the headers and rows ready to put into a table describing
   * the reference
   */
  function render_view($type = 'citation') {
    global $base_url;
    
    $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

    $headers = array('', '');
    $rows = array();

    $full_name = $this->topic;
    if (!empty($this->names)) {
      $names = implode(', ', $this->names);
    }
    else {
      $names = t('');
    }

    if (!empty($this->authors)) {
      $authors = implode(', ', $this->authors);
    }
    else {
      $authors = t('');
    }

    if (!empty($this->contributors)) {
      $contributors = implode(', ', $this->contributors);
    }
    else {
      $contributors = t('');
    }

    if (!empty($this->topic)) {
      $topics = implode($this->topic, ', ');
    }
    else {
      $topics = t('');
    }

    $genre = $this->genre;
    if (stripos((string) $genre[0], 'book') !== FALSE) {
      $type = 'book';
    }

    if ($this->embargo_date != NULL) {
      $embargo_date = date('r', tstamptotime((string) $this->embargo_date));
    }
    else {
      $embargo_date = NULL;
    }

    if ((string) $this->title != NULL) {
      $rows[] = array('Title', $this->title);
    }
    elseif (user_access('administer scholarly research')) {
      $rows[] = array('Title', $this->title);
    }

    if ($type == 'thesis') {
      if ($authors != NULL) {
        $rows[] = array('Authors', rtrim($authors, ', '));
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Authors', rtrim($authors, ', '));
      }
      if ($contributors != NULL) {
        $rows[] = array('Contributors', rtrim($contributors, ', '));
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Contributors', rtrim($contributors, ', '));
      }
      if (module_exists('scholar_tracking')) {
        $rows[] = array('URL', l($base_url . '/download_ds/' . $this->pid . '/PDF', $base_url . '/download_ds/' . $this->pid . '/PDF'));
      }
      else {
        $rows[] = array('URL', l($base_url . '/fedora/repository' . $this->pid . '/PDF/PDF', $base_url . '/fedora/repository' . $this->pid . '/PDF/PDF'));
      }
    }
    else {
      if ($names != NULL) {
        $rows[] = array('Authors', rtrim($names, ', '));
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Authors', rtrim($names, ', '));
      }
    }

    if ((string) $this->abstract != NULL) {
      $rows[] = array('Abstract', $this->abstract);
    }
    elseif (user_access('administer scholarly research')) {
      $rows[] = array('Abstract', $this->abstract);
    }

    if ($type == 'citation') {
      if ((string) $this->journal_title != NULL) {
        $rows[] = array('Journal', $this->journal_title);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Journal', $this->journal_title);
      }
    }

    if ((string) $this->date != NULL) {
      $rows[] = array('Date', $this->date);
    }
    elseif (user_access('administer scholarly research')) {
      $rows[] = array('Date', $this->date);
    }

    if ($type == 'citation') {
      if ((string) $this->volume != NULL) {
        $rows[] = array('Volume', $this->volume);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Volume', $this->volume);
      }

      if ((string) $this->issue != NULL) {
        $rows[] = array('Issue', $this->issue);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Issue', $this->issue);
      }

      if ((string) $this->start_page != NULL) {
        $rows[] = array('Start page', $this->start_page);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Start page', $this->start_page);
      }
    }

    if ($type == 'citation') {
      if ((string) $this->issn != NULL) {
        $rows[] = array('ISSN', $this->issn);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('ISSN', $this->issn);
      }

      if ((string) $topics != NULL) {
        $rows[] = array('Subjects', rtrim($topics, ', '));
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Subjects', rtrim($topics, ', '));
      }

      if ((string) $this->note != NULL) {
        $rows[] = array('Note', $this->note);
      }
      elseif (user_access('administer scholarly research')) {
        $rows[] = array('Note', $this->note);
      }
    }

    if ($embargo_date != NULL) {
      $rows[] = array('Embargo date', $embargo_date);
    }

    return array(
      'headers' => $headers,
      'rows' => $rows,
    );
  }

}