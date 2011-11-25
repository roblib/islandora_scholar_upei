<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
<xsl:variable name="OBJECTSPAGE">
		 	<xsl:value-of select="$objectsPage"/>	
</xsl:variable>
	<xsl:variable name='ORDERBY' select="$orderBy"/>
	<xsl:variable name="ALLOWEDNAMESPACES" select="$allowedPidNameSpaces"/>
    
<!--<xsl:variable name="PATHTOMAKEIMAGE">
		 	<xsl:value-of select="$pathToMakeImage"/>
		</xsl:variable>-->

<xsl:template match="gfindObjects">
<xsl:variable name="INDEXNAME" select="@indexName"/>

		<xsl:variable name="PREQUERY" select="substring-before(@query,':')"/>
		<xsl:variable name="QUERY" select="substring-after(@query,':')"/>
		<xsl:variable name="HITPAGESTART" select="@hitPageStart"/>
		<xsl:variable name="HITPAGESIZE" select="@hitTotal"/>
		<xsl:variable name="HITTOTAL" select="@hitTotal"/>
		
		<xsl:variable name="SEARCHURL">
		 	<xsl:value-of select="$searchUrl"/>
		</xsl:variable>

		<xsl:variable name="HITPAGEEND">
			<xsl:choose>
				<xsl:when test="$HITPAGESTART + $HITPAGESIZE - 1 > $HITTOTAL">
					<xsl:value-of select="$HITTOTAL"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="$HITPAGESTART + $HITPAGESIZE - 1"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<xsl:variable name="HITPAGENO" select="ceiling($HITPAGESTART div $HITPAGESIZE)"/>
		<xsl:variable name="HITPAGENOLAST" select="ceiling($HITTOTAL div $HITPAGESIZE)"/>
		<xsl:variable name="HITPAGESTARTT" select="(($HITPAGENO - 1) * $HITPAGESIZE + 1)"/>
<xsl:choose>
<xsl:when test="$HITTOTAL > 0">
		Total Hits = <strong><xsl:value-of select="@hitTotal"/></strong>
		<!-- Number of Hits/page = <xsl:value-of select="@hitPageSize"/>-->

		<style type="text/css">

		span.highlight
		{
		background-color:yellow
		}

		span.searchtitle
		{
		font-weight: bold
		}
</style>
<xsl:apply-templates select="objects"/>
</xsl:when>
	<xsl:otherwise>
	<div class="box">

  <h2>Your search yielded no results</h2>

  <div class="content"><ul>
<li>Check if your spelling is correct.</li>
</ul></div>

</div>

	</xsl:otherwise>
</xsl:choose>
</xsl:template>

<xsl:template match="objects">
    <table><div class="search-results">
    	<xsl:choose>
    		<xsl:when test="$ORDERBY = '2'">
    			<xsl:for-each select="object">    	 				
    				
    				<xsl:variable name="PIDVALUE">
    					<xsl:choose>
    						<xsl:when test="@PID">
    							<xsl:value-of select="@PID"/>
    						</xsl:when>
    						<xsl:when test="field[@name='PID' and @snippet='yes']">
    							<xsl:value-of select="field[@name='PID']/span/text()"/>
    						</xsl:when>
    						<xsl:otherwise>
    							<xsl:value-of select="field[@name='PID']/text()"/>
    						</xsl:otherwise>
    					</xsl:choose>
    				</xsl:variable>
    				
    				
    				<xsl:call-template name="splitBySpace">
    					<xsl:with-param name="PIDVALUE" select="$PIDVALUE"></xsl:with-param>
    					<xsl:with-param name="str" select="$ALLOWEDNAMESPACES"/>
    					
    				</xsl:call-template>
    				
    				
    			</xsl:for-each>
    		</xsl:when>
    		<xsl:otherwise>
    			<xsl:for-each select="object">    	
    				
    				<xsl:sort select="field[@name='refworks.yr']" order='descending'/>
    				
    				
    				<xsl:variable name="PIDVALUE">
    					<xsl:choose>
    						<xsl:when test="@PID">
    							<xsl:value-of select="@PID"/>
    						</xsl:when>
    						<xsl:when test="field[@name='PID' and @snippet='yes']">
    							<xsl:value-of select="field[@name='PID']/span/text()"/>
    						</xsl:when>
    						<xsl:otherwise>
    							<xsl:value-of select="field[@name='PID']/text()"/>
    						</xsl:otherwise>
    					</xsl:choose>
    				</xsl:variable>
    				
    				
    				<xsl:call-template name="splitBySpace">
    					<xsl:with-param name="PIDVALUE" select="$PIDVALUE"></xsl:with-param>
    					<xsl:with-param name="str" select="$ALLOWEDNAMESPACES"/>
    					
    				</xsl:call-template>
    				
    				
    			</xsl:for-each>
    		</xsl:otherwise>
    	</xsl:choose>
    	
    	
    
        </div></table>

</xsl:template>

	<xsl:template name="splitBySpace">
		<xsl:param name="str"/>
		<xsl:param name="PIDVALUE"/>
		
		
		<xsl:choose>
			<xsl:when test="contains($str,' ')">
				<!--'DO SOMETHING WITH THE VALUE IN
					{substring-before($str,' ')}-->
				<xsl:variable name="testString" select="substring-before($str,' ')"/>
				
				<xsl:if test="starts-with($PIDVALUE,$testString)">	
					
					<xsl:call-template name="showResult">
						
						<xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>	
						
					</xsl:call-template>
					
				</xsl:if>
				
				<!--<xsl:value-of	select="substring-before($str,' ')"/>-->
				<xsl:call-template name="splitBySpace">
					<xsl:with-param name="str"
						select="substring-after($str,' ')"/>
					<xsl:with-param name="PIDVALUE"
						select="$PIDVALUE"/>
					
					
				</xsl:call-template>
			</xsl:when>
			<xsl:otherwise>
			<!--	<xsl:if test="substring-before($str, ':') = substring-before($PIDVALUE, ':')">
					
					<xsl:call-template name="showResult">
						<xsl:with-param name="PIDVALUE" select="$PIDVALUE"/>
					</xsl:call-template>
					
				</xsl:if>-->
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

<xsl:template name="showResult">
	<xsl:param name="PIDVALUE"/>
	<xsl:variable name="TYPE" select="field[@name='refworks.rt']"/>
	<xsl:variable name="HASOBJDATASTREAM" select="field[@name='refworks.status']"/>
	
	<tr>

		<td width="95%" valign="top">
			<span class="searchtitle">
						<!--<xsl:value-of select="@no"/>
						<xsl:value-of select="'. '"/>-->
				
						<a>
							<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedorair/ir_full_record/<xsl:value-of select="$PIDVALUE"/>

							</xsl:attribute>
							<xsl:for-each select="field[@name='refworks.t1']">
								<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
							</xsl:for-each><xsl:text> </xsl:text>
							
						</a><br /></span><div><xsl:text>   </xsl:text>
							
							By: <xsl:text> </xsl:text>
							<xsl:for-each select="field[@name='refworks.a1']">
								<xsl:value-of select="." disable-output-escaping="yes"/>; <xsl:text> </xsl:text>
							</xsl:for-each><xsl:text> </xsl:text>
							<xsl:choose>
								<xsl:when test="$TYPE='Journal Article' or $TYPE='Magazine Article'">
									(<xsl:for-each select="field[@name='refworks.fd']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>									
									<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/>
									</xsl:for-each>)
							<xsl:for-each select="field[@name='refworks.jf']">
								<i><xsl:value-of select="." disable-output-escaping="yes"/>. </i><xsl:text> </xsl:text>
							</xsl:for-each><xsl:text> </xsl:text>							
							
							<xsl:for-each select="field[@name='refworks.vo']">
								Vol <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
							</xsl:for-each>
							<xsl:for-each select="field[@name='refworks.is']">
								Issue <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
							</xsl:for-each>
							<xsl:for-each select="field[@name='refworks.sp']">
								p. <xsl:value-of select="." disable-output-escaping="yes"/><xsl:text> </xsl:text>
							</xsl:for-each>
							<xsl:for-each select="field[@name='refworks.op']">
								- <xsl:text> </xsl:text><xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
							</xsl:for-each>	
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Book, Whole' or $TYPE='Book, Edited'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/>
									</xsl:for-each>).
									<xsl:for-each select="field[@name='refworks.pp']">
										<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Newspaper Article'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.fd']">
										<xsl:value-of select="." disable-output-escaping="yes"/>
									</xsl:for-each>)
									<xsl:for-each select="field[@name='refworks.t2']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>							
                                                                     <xsl:for-each select="field[@name='refworks.jf']">
                                                                                <xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
                                                                        </xsl:for-each><xsl:text> </xsl:text>
 	
									<xsl:for-each select="field[@name='refworks.vo']">
										Vol <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.is']">
										Issue <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.sp']">
										p. <xsl:value-of select="." disable-output-escaping="yes"/> - <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.op']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>	
									
								</xsl:when>
								<xsl:when test="$TYPE='Book, Section'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>)									
									<xsl:for-each select="field[@name='refworks.t2']">
										In:
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>							
									<xsl:for-each select="field[@name='refworks.a2']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<!--(<xsl:for-each select="field[@name='refworks.ed']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
										</xsl:for-each>).<xsl:text> </xsl:text>-->
									<xsl:if test="field[@name='refworks.pp']">
										(Eds.)
									</xsl:if>
									<xsl:for-each select="field[@name='refworks.pp']">
									
										<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.sp']">
										p. <xsl:value-of select="." disable-output-escaping="yes"/> - <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.op']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>	
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Conference Proceedings'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.fd']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).
									<xsl:if test="field[@name='refworks.a2']">
										In:
									</xsl:if>	
									<xsl:for-each select="field[@name='refworks.a2']">
										
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<xsl:if test="field[@name='refworks.t2']">
										(Eds.)
									</xsl:if>	
									<xsl:for-each select="field[@name='refworks.t2']">	
										
										<xsl:value-of select="." disable-output-escaping="yes"/><xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<xsl:for-each select="field[@name='refworks.jf']">
										<xsl:value-of select="." disable-output-escaping="yes"/><xsl:text> </xsl:text>
									</xsl:for-each>.<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.pp']">
										<xsl:value-of select="." disable-output-escaping="yes"/>:<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>	
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.sp']">
										p. <xsl:value-of select="." disable-output-escaping="yes"/>  <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.op']">
										- <xsl:text> </xsl:text><xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>	
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Dissertation/thesis'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>							
									Dissertation/thesis: 
									<xsl:for-each select="field[@name='refworks.ed']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.no']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Dissertation/Thesis, Unpublished'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.ed']">
										 <xsl:value-of select="." disable-output-escaping="yes"/>, <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>								
									<xsl:for-each select="field[@name='refworks.no']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									
									
								</xsl:when>
								
								
								<xsl:when test="$TYPE='Computer Program'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.pp']">
										<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>								
									<xsl:for-each select="field[@name='refworks.vo']">
										<xsl:value-of select="." disable-output-escaping="yes"/>. <xsl:text> </xsl:text>
									</xsl:for-each>
									
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Patent'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.is']">
										<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.no']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>								
									
									
								</xsl:when>
								
								<xsl:when test="$TYPE='Video/DVD'">
									(<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.pp']">
										<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>								
																		
								</xsl:when>
								
								<xsl:otherwise>
									(<xsl:for-each select="field[@name='refworks.fd']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.yr']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>).<xsl:text> </xsl:text>
									<xsl:for-each select="field[@name='refworks.t2']">
										<xsl:value-of select="." disable-output-escaping="yes"/>: <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.no']">
										<xsl:value-of select="." disable-output-escaping="yes"/>.<xsl:text> </xsl:text>
									</xsl:for-each><xsl:text> </xsl:text>								
									<xsl:for-each select="field[@name='refworks.ed']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:for-each select="field[@name='refworks.pp']">
										<xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
									</xsl:for-each>
									<xsl:if test="field[@name='refworks.pp']">
										:
									</xsl:if>
									
									<xsl:for-each select="field[@name='refworks.pb']">
										<xsl:value-of select="." disable-output-escaping="yes"/>. <xsl:text> </xsl:text>
									</xsl:for-each>
									
									
								</xsl:otherwise>
								
								
								</xsl:choose>
							<xsl:for-each select="field[@name='refworks.ab']">
								<br /><xsl:value-of select="." disable-output-escaping="yes"/> <xsl:text> </xsl:text>
							</xsl:for-each>	
								
								
							
							<!--<xsl:value-of select="field[@name='refworks.ab']/text()" disable-output-escaping="yes"/><xsl:text> </xsl:text>-->
						<!--this would have to be changed for different resolvers at different locations-->
                                                <xsl:variable name="BASEURL">
							http://articles.library.upei.ca:7888/godot/hold_tab.cgi?hold_tab_branch=PCU&amp;issn=<xsl:value-of select="field[@name='refworks.sn']/text()"/>&amp;date=<xsl:value-of select="field[@name='refworks.yr']/text()"/>&amp;volume=<xsl:value-of select="field[@name='refworks.vo']/text()"/>&amp;issue=<xsl:value-of select="field[@name='refworks.is']/text()"/>&amp;spage=<xsl:value-of select="field[@name='refworks.sp']/text()"/>&amp;atitle=<xsl:value-of select="field[@name='refworks.t1']/text()"/>&amp;stitle=<xsl:value-of select="field[@name='refworks.jf']/text()"/>
						</xsl:variable>
						
					<br />
			<xsl:choose>
				
							
							<xsl:when test="$HASOBJDATASTREAM = 'POST-PRINT'">
						<a>
							<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$PIDVALUE"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
								
							</xsl:attribute>
							Full Text [Post-Print]<br />
							
						</a>

			</xsl:when>
							<xsl:when test="$HASOBJDATASTREAM = 'PRE-PRINT'">	
								<a>
									<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$PIDVALUE"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
										
									</xsl:attribute>
									Full Text [Pre-Print]<br />
									
								</a>
								
							</xsl:when>
				<xsl:when test="$HASOBJDATASTREAM = 'PUBLISHED' or $HASOBJDATASTREAM='OTHER'" >	
								
								<a>
									<xsl:attribute name="href"><xsl:copy-of select="$OBJECTSPAGE"/>fedora/repository/<xsl:value-of select="$PIDVALUE"/>/OBJ/<xsl:value-of select="$HASOBJDATASTREAM"/>.pdf
										
									</xsl:attribute>
									Full Text <br />
									
								</a>
								
							</xsl:when>
							</xsl:choose>
								<br />
						</div>

					</td>


	</tr>
	<tr><td><xsl:text> </xsl:text></td></tr><!--whitespace-->


</xsl:template>

</xsl:stylesheet>
