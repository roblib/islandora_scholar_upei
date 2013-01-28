<div class="clear-block">
  <div id = "scholar-profile-content">
    <div class = "scholar-profile-left">
      <span class="scholar-tn"><?php print($tn_src); ?>
      </span>
      <?php print(theme('table', $profile_header, $profile_rows, array('class' => 'scholar-profile-table'))); ?>
    </div>
    <div class="scholar-profile-middle">
      <?php print($middle_content); ?>
      <br />
        <?php if (!empty($recent_citations) || $user->uid > 0) { ?>
        <h4>Recent Citations</h4>
        <span class="scholar-recent-pubs"><ul><?php print($recent_citations); ?></ul></span>
      <?php } ?>
      <br /><?php print($rss_feed); ?>
    </div>    
    <div class="scholar-profile-right">      
      <?php if (!empty($research_block) || $user->uid > 0) { ?>
      <h4>Research Interests</h4>
      <ul><?php print($research_block); ?></ul>
      <?php } ?>
      <?php if (!empty($grant_info) || $user->uid > 0) { ?>
      <h4>Grants</h4>
      <?php print($grant_info); 
      } ?>
      <?php foreach ($department_list as $key => $value) { ?>
        <h4>Other Scholars in <?php print($key); ?></h4>
        <ul><?php print($value); ?></ul>        
      <?php } ?>
    </div>
  </div>
</div>