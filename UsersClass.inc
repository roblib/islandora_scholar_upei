<?php

/**
 * @file
 * This is a collection of functions to handle getting
 * user info from the LDAP server and turning it into 
 * a useful form
 */
class UsersClass {

  public $givenname = NULL;
  public $familyname = NULL;
  public $mail = NULL;
  public $cn = NULL;
  public $uid = NULL;
  public $fullname = NULL;
  public $groupmembership = NULL;

  /**
   * Given a user this populates the object with details
   * from the LDAP entry
   * @param type $user 
   */
  public function __construct($user) {

    $ldap_host = variable_get('scholar_ldap_host', '');
    $ldap_port = variable_get('scholar_ldap_port', '389');

    $ds = ldap_connect($ldap_host, $ldap_port) or die("Could not connect to ldap host!");

    $basedn = variable_get('scholar_ldap_basedn', '');

    $filter = "(&(objectClass=person)(uid=*)(cn=$user))";

    $sr = ldap_list($ds, $basedn, $filter);

    $list = ldap_get_entries($ds, $sr);
    $this->uid = $list[0]['uid'][0];
    $this->cn = $list[0]['cn'][0];
    $this->fullname = $list[0]['fullname'][0];
    $this->mail = $list[0]['mail'][0];
    $this->groupmembership = $list[0]['groupmembership'];
    $this->givenname = $list[0]['givenname'][0];
    $this->familyname = $list[0]['sn'][0];
  }

  /**
   * This builds the MADS file from the LDAP data
   * @return DOMDocument 
   */
  public function ldap_to_mads() {
    $dom = new DOMDocument();

    $mads = $dom->createElementNS("http://www.loc.gov/mads/v2", 'mads');
    $dom->appendChild($mads);
    $mads->setAttribute('xmlns:xsi', 'http://www.w3.org/2001/XMLSchema-instance');
    $mads->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:xlink', 'http://www.w3.org/1999/xlink');
    $mads->setAttributeNS('http://www.w3.org/2000/xmlns/', 'xmlns:mads', 'http://www.loc.gov/mads/v2');
    $mads->setAttribute('xsi:schemaLocation', 'http://www.loc.gov/mads/v2 http://www.loc.gov/standards/mads/mads.xsd');

    $authority = $dom->createElement("authority");
    $mads->appendChild($authority);

    $name = $dom->createElement("name");
    $name->setAttribute("type", "personal");
    $authority->appendChild($name);

    $given = $dom->createElement("namePart", $this->givenname);
    $given->setAttribute("type", "given");
    $name->appendChild($given);

    $family = $dom->createElement("namePart", $this->familyname);
    $family->setAttribute("type", "family");
    $name->appendChild($family);

    $filename = drupal_get_path('module', 'scholar') . '/inclusionList.txt';
    $inclusionlist = file_get_contents($filename);
    $inclusionlist = fix_encoding($inclusionlist);
    $inclusionlist = explode(",", $inclusionlist);

    $related = $dom->createElement("related");
    $related->setAttribute("type", "other");
    $mads->appendChild($related);

    if (isset($this->groupmembership)) {
      foreach ($this->groupmembership as $group) {
        $dept = strtok($group, ',');
        $dept = ltrim($dept, 'cn=');
        $dept = islandora_scholar_convert_dept($dept);
        if (strlen($dept) > 1) {
          $topic = $dom->createElement("topic", $dept);
          $topic->setAttribute("authority", "department");
          $related->appendChild($topic);
        }
      }
    }

    $u1 = $dom->createElement("identifier", $this->cn);
    $u1->setAttribute("type", "u1");
    $mads->appendChild($u1);

    $affiliation = $dom->createElement("affiliation");
    $mads->appendChild($affiliation);

    $email = $dom->createElement("email", str_replace('groupwise.', '', $this->mail));
    $affiliation->appendChild($email);

//    $xml = $dom->saveXML();

    return $dom;
  }

  /**
   * Check whether the user already exists as an object in
   * the repo
   * @return type 
   */
  public function check_user() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    $user = $this->cn;
    $query_string = 'select $object $title from <#ri>
                             where ($object <dc:title>$title
                             and $object <fedora-model:hasModel> <info:fedora/ir:researcherCModel>
                             and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
                             and $object <mulgara:is> <info:fedora/ir:' . strtolower($user) . '>)
                             order by $title';
//var_dump($query_string);
    $query_string = htmlentities(urlencode($query_string));
    $content = '';
    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=1000&offset=0&lang=itql&stream=on&query=" . $query_string;
    $content .= do_curl($url);
    $result = new DOMDocument();
    $result->loadXML($content);
    $entry = $result->getElementsByTagName('title')->item(0)->nodeValue;
    $text = $result->saveXML();
//    var_dump($content);
    if (isset($entry)) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

}

function islandora_scholar_convert_dept($dept) {

  $find = array(
    'Robertson Library',
    'Anatomy Physiology',
    'Biology',
    'Business',
    'Chemistry',
    'Classics',
    'Companion Animals',
    'Computer Science and Info Technology',
    'Economics',
    'Education',
    'Engineering',
    'English',
    'Health Management',
    'History',
    'Home Economics',
    'Island Studies',
    'Mathematics and Statistics',
    'Modern Languages',
    'Music',
    'Nursing',
    'Path Micro',
    'Philosophy',
    'Physics',
    'Political Studies',
    'Psychology',
    'Religious Studies',
    'Soc Anth',
    'Womens Studies',
    'Blood',
  );
  $replace = array(
    'Robertson Library',
    'Biomedical Sciences',
    'Biology',
    'School of Business',//Is this correct?
    'Chemistry',
    'Classics',
    'Companion Animals',
    'Computer Science and Information Technology',
    'Economics',
    'Education',
    'Engineering',
    'English',
    'Health Management',
    'History',
    'Applied Human Sciences',//Change
    'Island Studies',
    'Mathematics and Statistics',
    'Modern Languages',
    'Music',
    'Nursing',
    'Pathology and Microbiology',
    'Philosophy',
    'Physics',
    'Political Science',
    'Psychology',
    'Religious Studies',
    'Sociology and Anthropology',
    'Women\'s Studies',
    'Blood',
    'School_of_Business', 
    );//Not sure about this one
  
  $dept = str_replace($find, $replace, $dept);
  drupal_set_message('Dept: ' . $dept);
  if (in_array($dept, $replace)) {
    drupal_set_message('Dept found');
    $new_dept = $dept;
  }
  else {
    $new_dept = NULL;
  }

  return $new_dept;
}