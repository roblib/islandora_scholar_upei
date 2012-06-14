<?php

/**
 * @file
 * Class to deal with tracking page loads on the scholar site
 */
class islandora_scholar_tracking {

  function __construct($pid) {
    $this->pid = $pid;
  }

  function add_page_count() {
    global $user;
    $ip_list = explode(' ', variable_get('scholar_tracking_ip_list', 'localhost 127.0.0.1'));
    $ip = $_SERVER['REMOTE_ADDR'];
    if (!in_array($ip, $ip_list)) {
      $result = db_query("INSERT INTO {islandora_scholar_page_tracking} (pid,time,user) VALUES ('%s', '%d', '%s')", $this->pid, time(), $user->name);
    }
    return $result;
  }

  function read_page_count() {
    $query = db_query("SELECT time FROM {islandora_scholar_page_tracking} WHERE pid = '%s'", $this->pid);
    while ($result = db_fetch_array($query)) {
      if ($result['time'] == '') {
        $array[] = 'Never';
      }
      else {
        $array[] = date('H:i:s Y-m-d', $result['time']);
      }
    }
    $array['count'] = count($array);
    return $array;
  }

  function add_ds_count($dsid) {
    global $user;
    $ip_list = explode(' ', variable_get('scholar_tracking_ip_list', 'localhost 127.0.0.1'));
    $ip = $_SERVER['REMOTE_ADDR'];
    if (!in_array($ip, $ip_list)) {
      $result = db_query("INSERT INTO {islandora_scholar_ds_tracking} (pid,dsid,time,user) VALUES ('%s', '%s', '%d', '%s')", $this->pid, $dsid, time(), $user->name);
    }
    return $result;
  }

  function read_ds_count($dsid) {
    $query = db_query("SELECT time FROM {islandora_scholar_ds_tracking} WHERE pid = '%s' AND dsid = '%s'", $this->pid, $dsid);
    while ($result = db_fetch_array($query)) {
      if ($result['time'] == '') {
        $array[] = 'Never';
      }
      else {
        $array[] = date('H:i:s Y-m-d', $result['time']);
      }
    }
    $array['count'] = count($array);
    return $array;
  }

}