<div id="clear-block">
  <b>Supplemental information</b></br>

    <table>
        <?php foreach ($supp_header as $header): ?>
        <th>
        <?php print $header ?>
        </th>
        <?php endforeach; ?>
    <?php foreach ($supp_rows as $row): ?>
      <tr class="odd">  
        <?php foreach ($row as $r): ?>

          <td><?php print $r ?></td>

        <?php endforeach; ?>
      </tr>
    <?php endforeach; ?>
  </table>
  
  <p>Name: <?php print l($zip_description, 'download_ds/' . $pid . '/ZIP') ?></br>
    <?php print t('Size: @size bytes', array('@size' => $zip_size))?></br>


</div>