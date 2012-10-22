<div id="clear-block">
  <table>
    <?php foreach ($rows as $row): ?>
      <tr class="odd">  
        <?php foreach ($row as $r): ?>

          <td><?php print $r ?></td>

        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>

  <h3><?php print t('Using %s citation style.', array('%s' => $csl)); ?></h3>

  <?php print $citation ?>


  <?php if (module_exists('scholar_tracking')): ?>
    </br><div>Times viewed: <?php print $times_read ?></div>  
  <?php endif; ?>

  <h3><?php print t('Adding this citation to "My List" will allow you to export this citation in other styles.'); ?></h3>

  <div class="scholar-fullview-btns">
    <?php if (isset($add_to_list)): ?>
      <?php print $add_to_list; ?>
    <?php endif; ?>
    <?php if (isset($full_text) && !$embargoed): ?>
      <?php print $full_text; ?>
    <?php endif; ?>
    <div class="scholar-refworks"><a href="http://rlproxy.upei.ca/login?url=http://www.refworks.com/express/ExpressImport.asp?vendor=Robertson+Library&filter=RIS%20Format&url=<?php print $base_url ?>/refworks_entry/<?php print urlencode($pid) ?>" target="RefWorksMain" class="button refworks">Export to RefWorks</A></div>
    <?php if (stripos((string) $genre, 'book') !== FALSE && (!isset($full_text) || $embargoed )): ?>
      <div class="scholar_coins_title">
          <span class="Z3988" title="&#10;&#9;&#9;&#9;&#9;ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Abook&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.isbn=<?php print $isbn ?>&amp;rft.date=<?php print $date ?>&amp;rft.title=<?php print $title ?>">
          </span>
      </div>
      <div class="scholar_coins_url">
        <a href="&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;isbn=<?php print $isbn ?>&date=<?php print $date ?>&title=<?php print $title ?>" target="_blank">Check for Full Text</a>
      </div>
    <?php endif; ?>
    <?php if (stripos((string) $genre, 'book') === FALSE && (!isset($full_text) || $embargoed )): ?>
      <div class="scholar_coins_title">
        <span class="Z3988" title="&#10;&#9;&#9;&#9;&#9;ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Ajournal&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.issn=<?php print $issn ?>&amp;rft.date=<?php print $date ?>&amp;rft.volume=<?php print $volume ?>&amp;rft.issue=<?php print $issue ?>&amp;rft.spage=<?php print $start_page ?>&amp;rft.atitle=<?php print $title ?>&amp;rft.jtitle=<?php print $journal_title ?>">
        </span>
      </div>
      <div class="scholar_coins_url">
        <a href="&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=<?php print $issn ?>&date=<?php print $date ?>&volume=<?php print $volume ?>&issue=<?php print $issue ?>&spage=<?php print $start_page ?>&atitle=<?php print $title ?>&stitle=<?php print $journal_title ?>" target="_blank">Check for Full Text</a>
      </div>
    <?php endif; ?>
    <?php if (isset($associate_button)): ?>
      <?php print $associate_button; ?>
    <?php endif; ?>
  </div>

</div>