<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">

<xsl:template match="reference">
<xsl:variable name="OBJECTSPAGE">
		 	<xsl:value-of select="$objectsPage"/>
</xsl:variable>
<xsl:variable name="PID">
		 	<xsl:value-of select="$pid"/>
</xsl:variable>
	<xsl:variable name="TYPE" select="rt"/>
	<xsl:variable name="HASOBJDATASTREAM" select="status"/>
	
	<table>
	<tr>

		<td width="95%" valign="top">
			<h4>
						
							<xsl:for-each select="t1">
								<xsl:value-of select="node()" disable-output-escaping="yes"/> <xsl:text> </xsl:text>
							</xsl:for-each><xsl:text> </xsl:text>
	
			</h4>
			
				
				By: <xsl:text> </xsl:text>
				<xsl:for-each select="a1">
					<xsl:value-of select="." disable-output-escaping="yes"/>; <xsl:text> </xsl:text>
				</xsl:for-each><xsl:text> </xsl:text>
				<xsl:choose>
					<xsl:when test="$TYPE='Journal Article' or $TYPE='Magazine Article'">
						(<xsl:for-each select="fd">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>									
						<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/>
						</xsl:for-each>)
						<xsl:for-each select="jf">
							<i><xsl:value-of select="." disable-output-escaping="yes"/>. </i><xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>							
						
						<xsl:for-each select="vo">
							Vol <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="is">
							Issue <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="sp">
							p. <xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="op">
							-  <xsl:text> </xsl:text><xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>	
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Book, Whole' or $TYPE='Book, Edited'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/>
						</xsl:for-each>).
						<xsl:for-each select="pp">
							<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Newspaper Article'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:text> </xsl:text>
						<xsl:for-each select="fd">
							<xsl:value-of select="." disable-output-escaping="yes"/>
						</xsl:for-each>)
						<xsl:for-each select="t2">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>							
                                                <xsl:for-each select="jf">
                                                        <xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
                                                </xsl:for-each><xsl:text> </xsl:text>
						
						<xsl:for-each select="vo">
							Vol <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="is">
							Issue <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="sp">
							p. <xsl:value-of select="." disable-output-escaping="yes"/> - <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="op">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>	
						
					</xsl:when>
					<xsl:when test="$TYPE='Book, Section'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>)	
						<xsl:if test="t2">
							In:
						</xsl:if>	
						<xsl:for-each select="t2">
							
							<i><xsl:value-of select="." disable-output-escaping="yes"/>.</i><xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>							
						<xsl:for-each select="a2">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						<!--(<xsl:for-each select="field[@name='refworks.ed']">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
							</xsl:for-each>).<xsl:text> </xsl:text>-->
						<xsl:text> </xsl:text>
						<xsl:for-each select="pp">
							(Eds.)
							<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>
						<xsl:for-each select="sp">
							p. <xsl:value-of select="." disable-output-escaping="yes"/> - <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="op">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>	
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Conference Proceedings'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="fd">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).
						<xsl:if test="a2">
							In:
						</xsl:if>						
						<xsl:for-each select="a2">								
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>
						<xsl:if test="t2">
							(Eds.)
						</xsl:if>
						<xsl:for-each select="t2">	
							
							<xsl:value-of select="." disable-output-escaping="yes"/><xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						<xsl:for-each select="jf">
							<xsl:value-of select="." disable-output-escaping="yes"/><xsl:text> </xsl:text>
						</xsl:for-each>.<xsl:text> </xsl:text>
						<xsl:for-each select="pp">
							<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>	
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>
						<xsl:for-each select="sp">
							p. <xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="op">
							- <xsl:text> </xsl:text><xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>	
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Dissertation/thesis'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>							
						Dissertation/thesis: 
						<xsl:for-each select="ed">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="no">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Dissertation/Thesis, Unpublished'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).<xsl:text> </xsl:text>
						<xsl:for-each select="ed">
							<xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>								
						<xsl:for-each select="no">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						
						
					</xsl:when>
					
					
					<xsl:when test="$TYPE='Computer Program'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).<xsl:text> </xsl:text>
						<xsl:for-each select="pp">
							<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>								
						<xsl:for-each select="vo">
							<xsl:value-of select="." disable-output-escaping="yes"/>. <xsl:text> </xsl:text>
						</xsl:for-each>
						
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Patent'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).<xsl:text> </xsl:text>
						<xsl:for-each select="is">
							<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="no">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>								
						
						
					</xsl:when>
					
					<xsl:when test="$TYPE='Video/DVD'">
						(<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).<xsl:text> </xsl:text>
						<xsl:for-each select="pp">
							<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>								
						
					</xsl:when>
					
					<xsl:otherwise>
						(<xsl:for-each select="fd">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="yr">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>).<xsl:text> </xsl:text>
						<xsl:for-each select="t2">
							<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="no">
							<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
						</xsl:for-each><xsl:text> </xsl:text>								
						<xsl:for-each select="ed">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:for-each select="pp">
							<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
						</xsl:for-each>
						<xsl:if test='pp'>
							:
						</xsl:if>
						
						<xsl:for-each select="pb">
							<xsl:value-of select="." disable-output-escaping="yes"/>. <xsl:text> </xsl:text>
						</xsl:for-each>
						
						
					</xsl:otherwise>
					
					
				</xsl:choose>	
			<xsl:text> </xsl:text> <br /><xsl:text> </xsl:text> <br />
			<xsl:variable name="BASEURL">
				http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=<xsl:value-of select="sn/text()"/>&amp;date=<xsl:value-of select="yr/text()"/>&amp;volume=<xsl:value-of select="vo/text()"/>&amp;issue=<xsl:value-of select="is/text()"/>&amp;spage=<xsl:value-of select="sp/text()"/>&amp;atitle=<xsl:value-of select="t1/text()"/>&amp;stitle=<xsl:value-of select="jf/text()"/>
			</xsl:variable>
			<xsl:variable name="COINS">
				ctx_ver=Z39.88-2004&amp;rft_val_fmt=info%3Aofi%2Ffmt%3Akev%3Amtx%3Ajournal&amp;rfr_id=info:sid/library.upei.ca:Robertson&amp;rft.issn=<xsl:value-of select="sn/text()"/>&amp;rft.date=<xsl:value-of select="yr/text()"/>&amp;rft.volume=<xsl:value-of select="vo/text()"/>&amp;rft.issue=<xsl:value-of select="is/text()"/>&amp;rft.spage=<xsl:value-of select="sp/text()"/>&amp;rft.atitle=<xsl:value-of select="t1/text()"/>&amp;rft.jtitle=<xsl:value-of select="jf/text()"/>
			</xsl:variable>
			
			
			<span>
				<xsl:if test="$HASOBJDATASTREAM = 'PUBLISHED' or $HASOBJDATASTREAM='OTHER'">	
					<a>
						<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$pid"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
							
						</xsl:attribute>
						<xsl:text> </xsl:text>Full Text <xsl:text> </xsl:text>   
						
						
					</a>		
					Use Permission: <xsl:for-each select="usage"> <xsl:value-of select="text()" disable-output-escaping="yes"/> 
					</xsl:for-each>
					
				</xsl:if>
				<xsl:if test="$HASOBJDATASTREAM = 'POST-PRINT'">	
					<a>
						<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$pid"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
							
						</xsl:attribute>
						<xsl:text> </xsl:text>Full Text [Post-Print]<xsl:text> </xsl:text>   
						
						
					</a>						
					
					Use Permission: <xsl:for-each select="usage"> <xsl:value-of select="text()" disable-output-escaping="yes"/> 
					</xsl:for-each><br />
					
					
				</xsl:if>
				
				<xsl:if test="$HASOBJDATASTREAM = 'PRE-PRINT'">	
					<a>
						<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$pid"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
							
						</xsl:attribute>
						<xsl:text> </xsl:text>Full Text [Pre-Print]<xsl:text> </xsl:text>   
						
						
					</a>					
					
					Use Permission: <xsl:for-each select="usage"> <xsl:value-of select="text()" disable-output-escaping="yes"/> 
					</xsl:for-each><br />
					
					
				</xsl:if>
				
				<!--<xsl:if test="$HASOBJDATASTREAM = 'NO_OBJ'">	-->
					<!--<strong>No local Full Text Available.</strong>--><xsl:text> </xsl:text> <xsl:text> </xsl:text>
					<br /><span class="Z3988" title="{$COINS}"><xsl:text> </xsl:text></span><xsl:text> </xsl:text><br />
					<a><xsl:attribute name="href"><xsl:copy-of select="$BASEURL"/>
						
					</xsl:attribute>
						<xsl:text> </xsl:text>UPEI Users Only: Check for Full Text<xsl:text> </xsl:text>   					
						
					</a>		
				<!--</xsl:if>-->
			</span><br />
			
			<xsl:text> </xsl:text> <br />
			
			
			
							<xsl:for-each select="ab">
								<h4>Abstract</h4>
								<xsl:value-of select="node()" disable-output-escaping="yes"/> <xsl:text> </xsl:text>
							</xsl:for-each>
							
							<!--<span ><xsl:value-of select="ab/text()" disable-output-escaping="yes"/><xsl:text> </xsl:text></span>-->
			<br />	
					</td>


	</tr>
	</table>
	<br />
	<!--<a><xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/ir_edit_refworks/<xsl:value-of select="$pid"/>
		
	</xsl:attribute>
		<xsl:text> </xsl:text>Edit Metadata<xsl:text> </xsl:text>   					
		
	</a>	-->


</xsl:template>
</xsl:stylesheet>
