<?php

class UsersClass {

  public $givenname = NULL;
  public $familyname = NULL;
  public $mail = NULL;
  public $cn = NULL;
  public $uid = NULL;
  public $fullname = NULL;
  public $groupmembership = NULL;

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

  public function ldap_to_mads() {
    $dom = new DOMDocument();

    $mads = $dom->createElementNS("http://www.loc.gov/mads/v2", 'mads');
    $dom->appendChild($mads);

    $authority = $dom->createElement("authority");
    $mads->appendChild($authority);

    $name = $dom->createElement("name");
    $authority->appendChild($name);

    $given = $dom->createElement("namePart", $this->givenname);
    $given->setAttribute("type", "given");
    $name->appendChild($given);

    $family = $dom->createElement("namePart", $this->familyname);
    $family->setAttribute("type", "family");
    $name->appendChild($family);

    if (isset($this->groupmembership)) {
      foreach ($this->groupmembership as $group) {
        $dept = ltrim(strstr($group, ',', TRUE), 'cn=');
        if ($dept != '') {
          $topic = $dom->createElement("topic", $dept);
          $topic->setAttribute("authority", "department");
          $authority->appendChild($topic);
        }
      }
    }

    $affiliation = $dom->createElement("affiliation");
    $mads->appendChild($affiliation);

    $email = $dom->createElement("email", $this->mail);
    $affiliation->appendChild($email);

//    $xml = $dom->saveXML();

    return $dom;
  }

  public function check_user() {
    module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
    $user = $this->cn;
    $pid = 'ir:2829';
    $query_string = 'select $object $title $content from <#ri>
                             where ($object <dc:title>$title
                             and $object <fedora-model:hasModel> $content
                             and $object <fedora-model:state> <info:fedora/fedora-system:def/model#Active>
                             and $title <mulgara:is> \'' . $user . '\')
                             minus $content <mulgara:is> <info:fedora/fedora-system:FedoraObject-3.0>
                             order by $title';

    $query_string = htmlentities(urlencode($query_string));

    $limit = '1000';
    $offset = '0';

    $content = '';
    $url = variable_get('fedora_repository_url', 'http://localhost:8080/fedora/risearch');
    $url .= "?type=tuples&flush=TRUE&format=Sparql&limit=1000&offset=0&lang=itql&stream=on&query=" . $query_string;
    $content .= do_curl($url);
//    drupal_set_message('Content: ' . $content);
    $result = new DOMDocument();
    $result->loadXML($content);
    $entry = $result->getElementsByTagName('title')->item(0)->nodeValue;
    $text = $result->saveXML();
    if (isset($entry)) {
      return TRUE;
    }
    else {
      return FALSE;
    }
  }

}