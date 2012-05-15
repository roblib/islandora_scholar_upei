<div id="clear-block">

  <h3><?php print t('Labelled view'); ?></h3>

  <table>
    <?php foreach ($rows as $row): ?>
      <tr class="odd">  
        <?php foreach ($row as $r): ?>

          <td><?php print $r ?></td>

        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>

  <h3><?php print t('Citation view'); ?></h3>

  <?php print $citation ?>

  </br><div>Times viewed: <?php print $times_read ?></div>
  <div>Last viewed: <?php print $time_last_viewed ?></div></br>  
  
  <div class="scholar_refworks_link"> <A href="http://www.refworks.com/express/ExpressImport.asp?vendor=Robertson+Library&filter=RIS%20Format&url=http%3A//<?php print $server_ip ?>/refworks_entry/<?php print urlencode($pid) ?>" target="RefWorksMain">Export to RefWorks</A></div>

  <div class="scholar_coins_title"><span>  <br /><span class="Z3988" title="&#10;&#9;&#9;&#9;&#9;ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Ajournal&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.issn=<?php print $issn ?>&amp;rft.date=<?php print $date ?>&amp;rft.volume=<?php print $volume ?>&amp;rft.issue=<?php print $issue ?>&amp;rft.spage=<?php print $start_page ?>&amp;rft.atitle=<?php print $title ?>&amp;rft.jtitle=<?php print $journal_title ?>"></span>

  </div><div><br/><A HREF="&#10;&#9;&#9;&#9;&#9;http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=<?php print $issn ?>&date=<?php print $date ?>&volume=<?php print $volume ?>&issue=<?php print $issue ?>&spage=<?php print $start_page ?>&atitle=<?php print $title ?>&stitle=<?php print $journal_title ?>" target="_blank">UPEI Users Only: Check for Full Text</A></span><br /></div>

</div>