<?php

/**
 * @file
 *  This is a mapping for translation from mods to json.
 */
$_ir_citation_marcgt_csl_terms = array(
  "article" => "article",
  "issue" => "article",
  "journal" => "article-journal",
  "periodical" => "article-journal",
  "newspaper" => "article-newspaper",
  "autobiography" => "book",
  "biography" => "book",
  "fiction" => "book",
  "history" => "book",
  "hymnal" => "book",
  "multivolume monograph" => "book",
  "novel" => "book",
  "book" => "book",
  "index" => "entry",
  "dictionary" => "entry-dictionary",
  "encyclopedia" => "entry-encyclopedia",
  "chart" => "figure",
  "graphic" => "graphic",
  "picture" => "graphic",
  "technical drawing" => "graphic",
  "interview" => "interview",
  "legal case and case notes" => "legal_case",
  "legislation" => "legislation",
  "atlas" => "map",
  "globe" => "map",
  "map" => "map",
  "motion picture" => "motion_picture",
  "videorecording" => "motion_picture",
  "conference publication" => "paper-conference",
  "patent" => "patent",
  "letter" => "personal_communication",
  "technical report" => "report",
  "discography" => "song",
  "speech" => "speech",
  "thesis" => "thesis",
  "online system or service" => "webpage",
  "web site" => "webpage",
  "abstract or summary" => NULL,
  "art original" => NULL,
  "art reproduction" => NULL,
  "bibliography" => NULL,
  "calendar" => NULL,
  "catalog" => NULL,
  "comic or graphic novel" => NULL,
  "comic strip" => NULL,
  "database" => NULL,
  "diorama" => NULL,
  "directory" => NULL,
  "drama" => NULL,
  "essay" => NULL,
  "festschrift" => NULL,
  "filmography" => NULL,
  "filmstrip" => NULL,
  "finding aid" => NULL,
  "flash card" => NULL,
  "folktale" => NULL,
  "font" => NULL,
  "game" => NULL,
  "government publication" => NULL,
  "handbook" => NULL,
  "humor, satire" => NULL,
  "instruction" => NULL,
  "kit" => NULL,
  "language instruction" => NULL,
  "law report or digest" => NULL,
  "legal article" => NULL,
  "loose-leaf" => NULL,
  "memoir" => NULL,
  "microscope slide" => NULL,
  "model" => NULL,
  "numeric data" => NULL,
  "offprint" => NULL,
  "poetry" => NULL,
  "programmed text" => NULL,
  "realia" => NULL,
  "rehearsal" => NULL,
  "remote sensing image" => NULL,
  "reporting" => NULL,
  "review" => 'review',
  "script" => NULL,
  "series" => NULL,
  "short story" => NULL,
  "slide" => NULL,
  "sound" => NULL,
  "standard or specification" => NULL,
  "statistics" => NULL,
  "survey of literature" => NULL,
  "toy" => NULL,
  "transparency" => NULL,
  "treaty" => 'treaty',
  "yearbook" => NULL,
);

$_ir_citation_zotero_local_csl_terms = array(
  "document" => NULL,
  "journalArticle" => "article-journal",
  "magazineArticle" => "article-magazine",
  "newspaperArticle" => "article-newspaper",
  "bill" => "bill",
  "book" => "book",
  "radioBroadcast" => "broadcast",
  "tvBroadcast" => "broadcast",
  "bookSection" => "chapter",
  "dictionaryEntry" => "entry-dictionary",
  "encyclopediaEntry" => "entry-encyclopedia",
  "artwork" => "graphic",
  "interview" => "interview",
  "case" => "legal_case",
  "manuscript" => "manuscript",
  "map" => "map",
  "film" => "motion_picture",
  "conferencePaper" => "paper-conference",
  "patent" => "patent",
  "email" => "personal_communication",
  "instantMessage" => "personal_communication",
  "letter" => "personal_communication",
  "report" => "report",
  "audioRecording" => "song",
  "podcast" => "song",
  "presentation" => "speech",
  "thesis" => "thesis",
  "blogPost" => "webpage",
  "forumPost" => "webpage",
  "webpage" => "webpage",
);

$_ir_citation_bibutils_noauth_csl_terms = array(
  "academic journal" => "article-journal",
  "magazine" => "article-magazine",
  "communication" => "personal_communication",
  "Ph.D. thesis" => "thesis",
  "Masters thesis" => "thesis",
  "Diploma thesis" => "thesis",
  "Doctoral thesis" => "thesis",
  "Habilitation thesis" => "thesis",
  "hearing" => NULL,
  "unpublished" => NULL,
);

function ir_citation_marcgt_csl_terms($term) {
  $input = array(
    "article",
    "issue",
    "journal",
    "periodical",
    "newspaper",
    "autobiography",
    "biography",
    "fiction",
    "history",
    "hymnal",
    "multivolume monograph",
    "novel",
    "book",
    "index",
    "dictionary",
    "encyclopedia",
    "chart",
    "graphic",
    "picture",
    "technical drawing",
    "interview",
    "legal case and case notes",
    "legislation",
    "atlas",
    "globe",
    "map",
    "motion picture",
    "videorecording",
    "conference publication",
    "patent",
    "letter",
    "technical report",
    "discography",
    "speech",
    "thesis",
    "treaty",
    "online system or service",
    "web site",
  );

  $output = array(
    "article",
    "article",
    "article-journal",
    "article-journal",
    "article-newspaper",
    "book",
    "book",
    "book",
    "book",
    "book",
    "book",
    "book",
    "book",
    "entry",
    "entry-dictionary",
    "entry-encyclopedia",
    "figure",
    "graphic",
    "graphic",
    "graphic",
    "interview",
    "legal_case",
    "legislation",
    "map",
    "map",
    "map",
    "motion_picture",
    "motion_picture",
    "paper-conference",
    "patent",
    "personal_communication",
    "report",
    "song",
    "speech",
    "thesis",
    "treaty",
    "webpage",
    "webpage",
  );

  $input2 = array(
    "abstract or summary",
    "art original",
    "art reproduction",
    "bibliography",
    "calendar",
    "catalog",
    "comic or graphic novel",
    "comic strip",
    "database",
    "diorama",
    "directory",
    "drama",
    "essay",
    "festschrift",
    "filmography",
    "filmstrip",
    "finding aid",
    "flash card",
    "folktale",
    "font",
    "game",
    "government publication",
    "handbook",
    "humor, satire",
    "instruction",
    "kit",
    "language instruction",
    "law report or digest",
    "legal article",
    "loose-leaf",
    "memoir",
    "microscope slide",
    "model",
    "numeric data",
    "offprint",
    "poetry",
    "programmed text",
    "realia",
    "rehearsal",
    "remote sensing image",
    "reporting",
    "review" => 'review',
    "script",
    "series",
    "short story",
    "slide",
    "sound",
    "standard or specification",
    "statistics",
    "survey of literature",
    "toy",
    "transparency",
    "yearbook",
  );

  if (in_array($term, $input)) {
    $output_value = str_replace($input, $output, $term);
  }
  elseif (in_array($term, $input2)) {
    $output_value = NULL;
  }
  else {
    $output_value = FALSE;
  }
  return $output_value;
}

function marcgt_to_csl($term) {
  global $_ir_citation_marcgt_csl_terms;
  if (!empty($_ir_citation_marcgt_csl_terms[$term])) {
    return $_ir_citation_marcgt_csl_terms[$term];
  }
  else {
    return FALSE;
  }
}

function zotero_to_csl($term) {
  global $_ir_citation_zotero_local_csl_terms;
  if (!empty($_ir_citation_zotero_local_csl_terms[$term])) {
    return $_ir_citation_zotero_local_csl_terms[$term];
  }
  else {
    return FALSE;
  }
}

function bibutils_to_csl($term) {
  global $_ir_citation_bibutils_noauth_csl_terms;
  if (!empty($_ir_citation_bibutils_noauth_csl_terms[$term])) {
    return $_ir_citation_bibutils_noauth_csl_terms[$term];
  }
  else {
    return FALSE;
  }
}

function local_to_csl($term) {
  $term = strtolower(trim($term));
  switch ($term) {
    case 'journal article':
    case 'journal, electronic':
    case 'newspaper article':
    case 'magazine article':
    case '':
      return 'article-journal';
    case 'book chapter':
    case 'book section':
    case 'book, section':
      return 'chapter';
    case 'book, whole':
      return 'book';
    case 'conference proceedings':
      return 'paper-conference';
    case 'dissertation/thesis':
    case 'signature project':
      return 'thesis';
    case 'web page':
      return 'webpage';
    default:
      return false;
  }
}

function mods_genre_to_csl_type($term) {
//  $marcgt = marcgt_to_csl($term);
  $marcgt = ir_citation_marcgt_csl_terms($term);
  if (!empty($marcgt))
  return $marcgt;

  $zotero = zotero_to_csl($term);
  if (!empty($zotero))
  return $zotero;

  $bibutils = bibutils_to_csl($term);
  if (!empty($bibutils))
    return $bibutils;
  $local = local_to_csl($term);
  if(!empty($local))
    return $local;
  
  return FALSE;
}