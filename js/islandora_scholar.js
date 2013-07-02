Drupal.behaviors.islandoraScholar = function(context) {
  // Hide all divs.
  function islandora_scholar_hide() {
    $('.scholar_choice').hide();
    $('.fedora_ingester').hide();
  }
  islandora_scholar_hide();
  
  // Disable normal click function.
  $('.scholar_button').click(function(event) {
    event.preventDefault();
  });
  
  $('#RIS').click(function() {
    islandora_scholar_hide();
    $('#bulk_ingest').show();
  });
  $('#PUBMD').click(function() {
    islandora_scholar_hide();
    $('#pubmed').show();
  });
    $('#single').click(function() {
    islandora_scholar_hide();
    $('.fedora_ingester').show();
  });
    $('#DOI').click(function() {
    islandora_scholar_hide();
    $('#doi').show();
  });
};