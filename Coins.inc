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
  public $sup_embargo_date = NULL;
  public $genre = NULL;
  public $family_names = NULL;
  public $full_names = NULL;
  public $username = NULL;
  public $department = NULL;
  public $type = NULL;
  public $status = NULL;
  public $isbn = NULL;
  public $object = NULL;

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
    $this->object = $object;
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
          else
          if ($name_part['type'] == 'family') {
            $names[$i] .= (string) rtrim($name_part, '.');
            $family_names[$i] = (string) rtrim($name_part, '.');
            $full_names[$i] .= ' ' . (string) rtrim($name_part, '.');
          }
          else {
            $names[$i] = trim($name_part);
          }
        }
      }
      $role = (string) $name->role->roleTerm;

      if ($role == 'aut' || $role == 'Author') {
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

    $form = $xml->physicalDescription->form;
    if (!empty($form)) {
      foreach ($xml->physicalDescription->form as $form) {
        if ($form['authority'] == 'local' && (string) $form != 'NO_OBJ') {
          $this->status = 'FULL_TEXT_AVAILABLE';
          $this->type = (string) $form;
        }
      }
    }


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
    $isbn = $xml->xpath('//mods:identifier[@type="isbn"]');
    $this->isbn = str_replace(array("&", "=", ",", ";"), array('', '', '', ''), (string) $isbn[0]);
    //$this->date = $xml->part->date;
    $date = $xml->xpath('//mods:originInfo/mods:dateIssued');
    if (isset($date[0])) {
      $this->date = $date[0];
    }
    $date = $xml->xpath('//mods:originInfo/mods:dateCreated');
    if ($this->date == NULL) {
      $this->date = $date[0];
    }
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

    $sup_embargo_date = $xml->xpath('//mods:dateOther[@type="supp_embargo"]');
    $this->sup_embargo_date = $sup_embargo_date[0];

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
    $modsxml = $this->mods;
    global $base_url;

    $fedora_url = variable_get('fedora_base_url', 'http://localhost:8080/fedora');

    $headers = array('', '');
    $rows = array();
    $genre = (string) $this->genre[0];
    if (!empty($genre)) {
      switch ((string) $genre) {
        case 'Journal Article':
          //journal stuff
          $this->get_title($rows, 'Title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Author(s)', $authors);
          }
          $journal = $modsxml->xpath('/mods:mods/mods:relatedItem/mods:titleInfo[not(@type="abbreviated")]/mods:title');
          $journal = implode(', ', $journal);
          if (!empty($journal) && strlen(trim($journal)) > 1) {
            $rows[] = array('Journal', $journal);
          }

          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          if (isset($this->volume)) {
            $rows[] = array('Volume', $this->volume);
          }

          if (isset($this->issue)) {
            $rows[] = array('Issue', $this->issue);
          }

          if (!empty($this->start_page)) {
            $rows[] = array('Start page', $this->start_page);
          }
          $endp = $modsxml->xpath('//mods:extent[@unit="page"]/mods:end');
          $endp = implode(',', $endp);
          if (!empty($endp) && strlen(trim($endp)) > 1) {
            $rows[] = array('End page', $endp);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $doi = $modsxml->xpath('//mods:identifier[@type="doi"]');
          $doi = implode(',', $doi);
          if (!empty($doi) && strlen(trim($doi)) > 1) {
            $rows[] = array('DOI', l($doi, 'http://dx.doi.org/' . $doi));
          }
          $issn = $modsxml->xpath('//mods:identifier[@type="issn"]');
          $issn = implode(',', $issn);
          if (!empty($issn) && strlen(trim($issn)) > 1) {
            $rows[] = array('ISSN', $issn);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        case 'Book, Whole':
          $this->get_title($rows, 'Title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Author(s)', $authors);
          }
          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Publisher', $publishers);
          }
          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }
          $pages = $modsxml->xpath('/mods:mods/mods:physicalDescription/mods:extent');
          $pages = implode(', ', $pages);
          if (!empty($pages) && strlen(trim($pages)) > 1) {
            $rows[] = array('Pages', $pages);
          }
          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        case 'Book, Section':
          $this->get_title($rows, 'Section title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors)) {
            $rows[] = array('Section author(s)', $authors);
          }
          $btitle = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:title');
          $btitle = (string) $btitle[0];
          $bsub_title = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:subTitle');
          $bsub_title = (string) $bsub_title[0];
          if (!empty($bsub_title)) {
            $btitle = $btitle . ': ' . $bsub_title;
          }
          if (!empty($btitle)) {
            $rows[] = array('Book title', $btitle);
          }

          $beditors = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:name');
          $beditors_arr = array();
          foreach ($beditors as $beditor) {
            $value = trim((string) $beditor->namePart[0] . ' ' . (string) $beditor->namePart[1]);
            if (!empty($value)) {
              $beditors_arr[] = $value;
            }
          }
          $editor_name = implode(', ', $beditors_arr);
          if (!empty($editor_name)) {
            $rows[] = array('Book editor(s)', $editor_name);
          }
          if (!empty($this->start_page)) {
            $rows[] = array('Start page', $this->start_page);
          }
          $endp = $modsxml->xpath('//mods:extent[@unit="page"]/mods:end');
          $endp = implode(',', $endp);
          if (!empty($endp) && strlen(trim($endp)) > 1) {
            $rows[] = array('End page', $endp);
          }

          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Publisher', $publishers);
          }
          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }

          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        case 'thesis':
        case 'Signature Project':
          $this->get_title($rows, 'Title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Author(s)', $authors);
          }
          $btitle = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:title');
          $btitle = (string) $btitle[0];
          $bsub_title = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:subTitle');
          $bsub_title = (string) $bsub_title[0];
          if (!empty($bsub_title)) {
            $btitle = $btitle . ': ' . $bsub_title;
          }
          if (!empty($btitle)) {
            $rows[] = array('Related item', $btitle);
          }


          if (!empty($this->start_page)) {
            $rows[] = array('Start page', $this->start_page);
          }
          $endp = $modsxml->xpath('//mods:extent[@unit="page"]/mods:end');
          $endp = implode(',', $endp);
          if (!empty($endp) && strlen(trim($endp)) > 1) {
            $rows[] = array('End page', $endp);
          }

          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Institution', $publishers);
          }
          //advisor
          $advisors = $modsxml->xpath('/mods:mods/mods:name');
          $i = 0;
          $advisor_name = '';
          foreach ($advisors as $advisor) {
            if ($advisor->role->roleTerm == "Signature Project Advisor" || $advisor->role->roleTerm == "Thesis advisor") {
              if ($i > 0) {
                $advisor_name .= ', ';
              }
              $advisor_name .= (string) $advisor->namePart[0] . ' ' . (string) $advisor->namePart[1] . ' ' . (string) $advisor->namePart[2];
              $advisor_name = trim($advisor_name);
              if (!empty($advisor_name)) {
                $i++;
              }
            }
          }

          if (!empty($advisor_name)) {
            $advisor_name = trim($advisor_name);
            $rows[] = array('Advisor(s)', $advisor_name);
          }

          $i = 0;
          $member_name = '';
          foreach ($advisors as $member) {

            if ($member->role->roleTerm == "Committee member") {
              if ($i > 0) {
                $member_name .= ', ';
              }
              $member_name .= (string) $member->namePart[0] . ' ' . (string) $member->namePart[1] . ' ' . (string) $member->namePart[2];
              $member_name = trim($member_name);
              if (!empty($member_name)) {
                $i++;
              }
            }
          }

          if (!empty($member_name)) {
            $member_name = trim($member_name);
            $rows[] = array('Committee member(s)', $member_name);
          }
          $modsxml->registerXPathNamespace('etd', 'http://www.ndltd.org/standards/metadata/etdms/1.0');
          $degree_name = $modsxml->xpath('//etd:name');
          $degree_name = (string) $degree_name[0];
          $degree_level = $modsxml->xpath('//etd:level');
          $degree_level = (string) $degree_level[0];
          if (!empty($degree_name) && !empty($degree_level)) {
            $rows[] = array('Degree earned', $degree_name);
          }

          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }

          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        case 'Conference Proceedings':
          $this->get_title($rows, 'Paper title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Paper author(s)', $authors);
          }
          $pro_title = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:title');
          $pro_sub_title = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:subTitle');
          $pro_title = (string) $pro_title[0]; //implode(', ', $pro_title);
          $pro_sub_title = (string) $pro_sub_title[0]; //implode(',', $pro_sub_title);
          if (!empty($pro_sub_title[0])) {
            $pro_title = $pro_title . ': ' . $pro_sub_title;
          }
          if (!empty($pro_title)) {
            $rows[] = array('Proceedings title', $pro_title);
          }
          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $beditors = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:name/mods:namePart');
          $beditors = implode(', ', $beditors);
          if (!empty($beditors) && strlen(trim($beditors)) > 1) {
            $rows[] = array('Conference', $beditors);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Publisher', $publishers);
          }
          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }
          $pages = $modsxml->xpath('/mods:mods/mods:physicalDescription/mods:extent');
          $pages = implode(', ', $pages);
          if (!empty($pages) && strlen(trim($pages)) > 1) {
            $rows[] = array('Pages', $pages);
          }
          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        case 'Book, Edited':
          $this->get_title($rows, 'Title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Editor(s)', $authors);
          }
          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Publisher', $publishers);
          }
          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }
          $pages = $modsxml->xpath('/mods:mods/mods:physicalDescription/mods:extent');
          $pages = implode(', ', $pages);
          if (!empty($pages) && strlen(trim($pages)) > 1) {
            $rows[] = array('Pages', $pages);
          }
          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);
          break;
        default:
          $this->get_title($rows, 'Title');
          $authors = implode(', ', $this->authors);
          if (!empty($authors) && strlen(trim($authors)) > 1) {
            $rows[] = array('Author(s)', $authors);
          }
          $btitle = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:title');
          $btitle = (string) $btitle[0];
          $bsub_title = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:subTitle');
          $bsub_title = (string) $bsub_title[0];
          if (!empty($bsub_title)) {
            $btitle = $btitle . ': ' . $bsub_title;
          }
          if (!empty($btitle)) {
            $rows[] = array('Related item', $btitle);
          }

          $beditors = $modsxml->xpath('/mods:mods/mods:relatedItem[@type="host"]/mods:name/mods:namePart');
          $beditors = implode(', ', $beditors);
          if (!empty($beditors) && strlen(trim($beditors)) > 1) {
            $rows[] = array('Editors', $beditors);
          }
          if (!empty($this->start_page)) {
            $rows[] = array('Start page', $this->start_page);
          }
          $endp = $modsxml->xpath('//mods:extent[@unit="page"]/mods:end');
          $endp = implode(',', $endp);
          if (!empty($endp) && strlen(trim($endp)) > 1) {
            $rows[] = array('End page', $endp);
          }

          if ((string) $this->date != NULL) {
            $rows[] = array('Date', $this->date);
          }
          $publishers = $modsxml->xpath('/mods:mods/mods:originInfo/mods:publisher');
          $publishers = implode(', ', $publishers);
          if (!empty($publishers) && strlen(trim($publishers)) > 1) {
            $rows[] = array('Publisher', $publishers);
          }
          $place = $modsxml->xpath('/mods:mods/mods:originInfo/mods:place/mods:placeTerm');
          $place = implode(', ', $place);
          if (!empty($place) && strlen(trim($place)) > 1) {
            $rows[] = array('Place published', $place);
          }

          $isbn = $modsxml->xpath('/mods:identifier[@type="isbn"]');
          $isbn = implode(', ', $isbn);
          if (!empty($isbn) && strlen(trim($isbn)) > 1) {
            $rows[] = array('ISBN', $isbn);
          }
          if (!empty($this->abstract)) {
            $rows[] = array('Abstract', $this->abstract);
          }
          $this->get_availability($rows);
          $this->get_use($rows);

          break;
      }
    }

    $ds_array = $this->object->get_datastreams_list_as_array();
    $check = NULL;
    if (array_key_exists('OBJ', $ds_array)) {//check status in case ds exists but is embargoed
      $check = 'OBJ';
    }
    if (array_key_exists('PDF', $ds_array)) {
      $check = 'PDF';
    }
    $embargoed = $this->is_embargoed($this->embargo_date);
    if ($check && !$embargoed) {
      if (module_exists('scholar_tracking')) {
        //check to see if the full text is available before we provide a URL
        $rows[] = array('Text', l($base_url . '/download_ds/' . $this->pid . '/' . $check, $base_url . '/download_ds/' . $this->pid . '/' . $check));
      }
      else {
        $rows[] = array('Text', l($base_url . '/fedora/repository/' . $this->pid . '/' . $check . '/' . $this->pid, $base_url . '/fedora/repository/' . $this->pid . '/' . $check . '/PDF'));
      }
    }
    return array(
      'headers' => $headers,
      'rows' => $rows,
    );
  }

  function get_title(&$rows, $label) {
    $title = $this->mods->xpath('/mods:mods/mods:titleInfo/mods:title');
    $title = (string) $title[0];
    $sub_title = $this->mods->xpath('/mods:mods/mods:titleInfo/mods:subTitle');
    $sub_title = (string) $sub_title[0];
    if (!empty($sub_title)) {
      $title = $title . ': ' . $sub_title;
    }
    if (!empty($title)) {
      $rows[] = array($label, $title);
    }
  }

  /**
   *  gets the title we want to send to godot (UPEI resolver)
   * we send the host title if it is a book chapter or section
   * otherwise its the normal title
   * @return string
   */
  function get_godot_title() {
    $title = $this->title;
    if ($this->genre[0] == 'Book, Section' || $this->genre[0] == 'Book, Chapter') {
      module_load_include('php', 'citeproc', 'generators/converter.php');
      $title = convert_mods_to_citeproc_json_title($this->mods, '/mods:mods/mods:relatedItem[@type="host"]/mods:titleInfo/mods:title') ;
    }
    return $title;
  }

  function get_use(&$rows) {
    $use = $this->mods->xpath('/mods:mods/mods:accessCondition[@type="use and reproduction"]');
    $use = implode('<br /><br /> ', $use);
    $search_arr = array('Contact Author', 'Contact Publisher');
    $replace_arr = array('<a href="mailto:publications@upei.ca?subject=Scholar Contact request for object ' . $this->pid . '">Contact Author</a>', '<a href="mailto:publications@upei.ca?subject=' . $pid . '">Contact Publisher</a>');
    $use = str_replace($search_arr, $replace_arr, trim($use));
    if (!empty($use)) {
      $rows[] = array('Use/Reproduction', $use);
    }
  }

  function get_availability(&$rows) {
    if ($this->embargo_date != NULL && $this->is_embargoed($this->embargo_date)) {
      $embargo_date = date('r', tstamptotime((string) $this->embargo_date));
      $rows[] = array('Availability', 'This item is not yet available online, but may be available on request. Please contact the
<a href="mailto:publications@upei.ca?subject=IslandScholar Request">Robertson Library</a> if you wish to access this document. ' . $embargo_date);
    }
  }

  function is_embargoed($embargo_date) {
    if ($embargo_date === NULL) {
      return FALSE;
    }
    $embargo_timestamp = strtotime($embargo_date);
    if (!$embargo_timestamp) {
      return FALSE;
    }
    $today = time();
    $embargoed = FALSE;
    if ($embargo_timestamp > $today) {
      $embargoed = TRUE;
    }
    return $embargoed;
  }

}