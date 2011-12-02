/*
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights
 * Reserved.
 *
 * The contents of this file are subject to the Common Public
 * Attribution License Version 1.0 (the “License”); you may not use
 * this file except in compliance with the License. You may obtain a
 * copy of the License at:
 *
 * http://bitbucket.org/fbennett/citeproc-js/src/tip/LICENSE.
 *
 * The License is based on the Mozilla Public License Version 1.1 but
 * Sections 14 and 15 have been added to cover use of software over a
 * computer network and provide for limited attribution for the
 * Original Developer. In addition, Exhibit A has been modified to be
 * consistent with Exhibit B.
 *
 * Software distributed under the License is distributed on an “AS IS”
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is the citation formatting software known as
 * "citeproc-js" (an implementation of the Citation Style Language
 * [CSL]), including the original test fixtures and software located
 * under the ./std subdirectory of the distribution archive.
 *
 * The Original Developer is not the Initial Developer and is
 * __________. If left blank, the Original Developer is the Initial
 * Developer.
 *
 * The Initial Developer of the Original Code is Frank G. Bennett,
 * Jr. All portions of the code written by Frank G. Bennett, Jr. are
 * Copyright (c) 2009 and 2010 Frank G. Bennett, Jr. All Rights Reserved.
 *
 * Alternatively, the contents of this file may be used under the
 * terms of the GNU Affero General Public License (the [AGPLv3]
 * License), in which case the provisions of [AGPLv3] License are
 * applicable instead of those above. If you wish to allow use of your
 * version of this file only under the terms of the [AGPLv3] License
 * and not to allow others to use your version of this file under the
 * CPAL, indicate your decision by deleting the provisions above and
 * replace them with the notice and other provisions required by the
 * [AGPLv3] License. If you do not delete the provisions above, a
 * recipient may use your version of this file under either the CPAL
 * or the [AGPLv3] License.”
 */

/**
 * A Javascript implementation of the CSL citation formatting language.
 *
 * <p>A configured instance of the process is built in two stages,
 * using {@link CSL.Core.Build} and {@link CSL.Core.Configure}.
 * The former sets up hash-accessible locale data and imports the CSL format file
 * to be applied to the citations,
 * transforming it into a one-dimensional token list, and
 * registering functions and parameters on each token as appropriate.
 * The latter sets jump-point information
 * on tokens that constitute potential branch
 * points, in a single back-to-front scan of the token list.
 * This
 * yields a token list that can be executed front-to-back by
 * body methods available on the
 * {@link CSL.Engine} class.</p>
 *
 * <p>This top-level {@link CSL} object itself carries
 * constants that are needed during processing.</p>
 * @namespace A CSL citation formatter.
 */

// IE6 does not implement Array.indexOf().
// IE7 neither, according to rumour.

if (!Array.indexOf) {
	Array.prototype.indexOf = function (obj) {
		var i, len;
		for (i = 0, len = this.length; i < len; i += 1) {
			if (this[i] === obj) {
				return i;
			}
		}
		return -1;
	};
}

var CSL = {

	GENDERS: ["masculine", "feminine"],
	
	ERROR_NO_RENDERED_FORM: 1,

	PREVIEW: "Just for laughs.",
	ASSUME_ALL_ITEMS_REGISTERED: 2,

	START: 0,
	END: 1,
	SINGLETON: 2,

	SEEN: 6,
	SUCCESSOR: 3,
	SUCCESSOR_OF_SUCCESSOR: 4,
	SUPPRESS: 5,

	SINGULAR: 0,
	PLURAL: 1,

	LITERAL: true,

	BEFORE: 1,
	AFTER: 2,

	DESCENDING: 1,
	ASCENDING: 2,

	ONLY_FIRST: 1,
	ALWAYS: 2,
	ONLY_LAST: 3,

	FINISH: 1,

	POSITION_FIRST: 0,
	POSITION_SUBSEQUENT: 1,
	POSITION_IBID: 2,
	POSITION_IBID_WITH_LOCATOR: 3,

	MARK_TRAILING_NAMES: true,

	POSITION_TEST_VARS: ["position", "first-reference-note-number", "near-note"],

	AREAS: ["citation", "citation_sort", "bibliography", "bibliography_sort"],

	MULTI_FIELDS: ["publisher", "publisher-place", "title","container-title", "collection-title", "institution", "authority","edition"],

	CITE_FIELDS: ["first-reference-note-number", "locator"],

	MINIMAL_NAME_FIELDS: ["literal", "family"],

	SWAPPING_PUNCTUATION: [".", "!", "?", ":",",",";"],
	TERMINAL_PUNCTUATION: [".", "!", "?", ":", " "],
	SPLICE_PUNCTUATION: [".", "!", "?", ":", ";", ","],

	// update modes
	NONE: 0,
	NUMERIC: 1,
	POSITION: 2,

	COLLAPSE_VALUES: ["citation-number", "year", "year-suffix"],

	DATE_PARTS: ["year", "month", "day"],
	DATE_PARTS_ALL: ["year", "month", "day", "season"],
	DATE_PARTS_INTERNAL: ["year", "month", "day", "year_end", "month_end", "day_end"],

	NAME_PARTS: ["family", "given", "dropping-particle", "non-dropping-particle", "suffix", "literal"],
	DECORABLE_NAME_PARTS: ["given", "family", "suffix"],

	// XXXX: Apparently never used
	// ET_AL_NAMES: [
	// 	"et-al-min",
	// 	"et-al-use-first",
	// 	"et-al-use-last",
	// 	"et-al-subsequent-min",
	// 	"et-al-subsequent-use-first"
	// ],

	DISAMBIGUATE_OPTIONS: [
		"disambiguate-add-names",
		"disambiguate-add-givenname",
		"disambiguate-add-year-suffix"
	],

	GIVENNAME_DISAMBIGUATION_RULES: [
		"all-names",
		"all-names-with-initials",
		"primary-name",
		"primary-name-with-initials",
		"by-cite"
	],

	NAME_ATTRIBUTES: [
		"and",
		"delimiter-precedes-last",
		"delimiter-precedes-et-al",
		"initialize-with",
		"name-as-sort-order",
		"sort-separator",
		"et-al-min",
		"et-al-use-first",
		"et-al-subsequent-min",
		"et-al-subsequent-use-first"
	],

	PARALLEL_MATCH_VARS: ["container-title"],
	PARALLEL_TYPES: ["legal_case",  "legislation"],
	PARALLEL_COLLAPSING_MID_VARSET: ["volume", "container-title", "section"],

	LOOSE: 0,
	STRICT: 1,

	PREFIX_PUNCTUATION: /[.;:]\s*$/,
	SUFFIX_PUNCTUATION: /^\s*[.;:,\(\)]/,

	NUMBER_REGEXP: /(?:^\d+|\d+$)/,
	QUOTED_REGEXP_START: /^"/,
	QUOTED_REGEXP_END: /^"$/,
	//
	// \u0400-\u042f are cyrillic and extended cyrillic capitals
	// this is not fully smart yet.  can't do what this was trying to do
	// with regexps, actually; we want to identify strings with a leading
	// capital letter, and any subsequent capital letters.  Have to compare
	// locale caps version with existing version, character by character.
	// hard stuff, but if it breaks, that's what to do.
	NAME_INITIAL_REGEXP: /^([A-Z\u0080-\u017f\u0400-\u042f])([a-zA-Z\u0080-\u017f\u0400-\u052f]*|)/,
	ROMANESQUE_REGEXP: /[a-zA-Z\u0080-\u017f\u0400-\u052f\u0386-\u03fb\u1f00-\u1ffe]/,
	STARTSWITH_ROMANESQUE_REGEXP: /^[&a-zA-Z\u0080-\u017f\u0400-\u052f\u0386-\u03fb\u1f00-\u1ffe]/,
	ENDSWITH_ROMANESQUE_REGEXP: /[&a-zA-Z\u0080-\u017f\u0400-\u052f\u0386-\u03fb\u1f00-\u1ffe]$/,
	ALL_ROMANESQUE_REGEXP: /^[a-zA-Z\u0080-\u017f\u0400-\u052f\u0386-\u03fb\u1f00-\u1ffe]+$/,

	VIETNAMESE_SPECIALS: /[\u00c0-\u00c3\u00c8-\u00ca\u00cc\u00cd\u00d2-\u00d5\u00d9\u00da\u00dd\u00e0-\u00e3\u00e8-\u00ea\u00ec\u00ed\u00f2-\u00f5\u00f9\u00fa\u00fd\u0101\u0103\u0110\u0111\u0128\u0129\u0168\u0169\u01a0\u01a1\u01af\u01b0\u1ea0-\u1ef9]/,

	VIETNAMESE_NAMES: /^(?:(?:[.AaBbCcDdEeGgHhIiKkLlMmNnOoPpQqRrSsTtUuVvXxYy \u00c0-\u00c3\u00c8-\u00ca\u00cc\u00cd\u00d2-\u00d5\u00d9\u00da\u00dd\u00e0-\u00e3\u00e8-\u00ea\u00ec\u00ed\u00f2-\u00f5\u00f9\u00fa\u00fd\u0101\u0103\u0110\u0111\u0128\u0129\u0168\u0169\u01a0\u01a1\u01af\u01b0\u1ea0-\u1ef9]{2,6})(\s+|$))+$/,

	NOTE_FIELDS_REGEXP: /{:[-a-z]+:[^}]+}/g,
	NOTE_FIELD_REGEXP: /{:([-a-z]+):([^}]+)}/,

	DISPLAY_CLASSES: ["block", "left-margin", "right-inline", "indent"],

	NAME_VARIABLES: [
		"author",
		"editor",
		"translator",
		"contributor",
		"collection-editor",
		"composer",
		"container-author",
		"editorial-director",
		"interviewer",
		"original-author",
		"recipient"
	],
	NUMERIC_VARIABLES: ["edition", "volume", "number-of-volumes", "number", "issue", "citation-number"],
	//var x = new Array();
	//x = x.concat(["title","container-title","issued","page"]);
	//x = x.concat(["locator","collection-number","original-date"]);
	//x = x.concat(["reporting-date","decision-date","filing-date"]);
	//x = x.concat(["revision-date"]);
	//NUMERIC_VARIABLES = x.slice();
	DATE_VARIABLES: ["issued", "event-date", "accessed", "container", "original-date"],

	// TAG_ESCAPE: /(<span class=\"no(?:case|decor)\">.*?<\/span>)/,
	TAG_ESCAPE: function (str) {
		var mx, lst, len, pos, m, buf1, buf2, idx, ret, myret;
		// Workaround for Internet Exporer
		mx = str.match(/(<span\s+class=\"no(?:case|decor)\">)/g);
		lst = str.split(/<span\s+class=\"no(?:case|decor)\">/g);
		myret = [lst[0]];
		for (pos = 1, len = lst.length; pos < len; pos += 1) {
			myret.push(mx[pos - 1]);
			myret.push(lst[pos]);
		}
		lst = myret.slice();
		len = lst.length - 1;
		for (pos = len; pos > 1; pos += -2) {
			m = lst[pos].match(/<\/span>/);
			if (m) {
				idx = lst[pos].indexOf("</span>");
				buf1 = lst[pos].slice(0, idx);
				buf2 = lst[pos].slice(idx + 7);
				lst[pos - 1] += buf1 + "</span>";
				lst[pos] = buf2;
			} else {
				buf1 = lst.slice(0, pos - 1);
				if (pos < (lst.length - 1)) {
					buf2 = lst[pos - 1] + lst[pos];
				} else {
					buf2 = lst[pos - 1] + lst[pos] + lst[pos + 1];
				}
				lst = buf1.push(buf2).concat(lst.slice(pos + 2));
			}
		}
		return lst;
	},

	// TAG_USEALL: /(<[^>]+>)/,
	TAG_USEALL: function (str) {
		var ret, open, close, end;
		ret = [""];
		open = str.indexOf("<");
		close = str.indexOf(">");
		while (open > -1 && close > -1) {
			if (open > close) {
				end = open + 1;
			} else {
				end = close + 1;
			}
			if (open < close && str.slice(open + 1, close).indexOf("<") === -1) {
				ret[ret.length - 1] += str.slice(0, open);
				ret.push(str.slice(open, close + 1));
				ret.push("");
				str = str.slice(end);
			} else {
				ret[ret.length - 1] += str.slice(0, close + 1);
				str = str.slice(end);
			}
			open = str.indexOf("<");
			close = str.indexOf(">");
		}
		ret[ret.length - 1] += str;
		return ret;
	},


	SKIP_WORDS: ["a", "the", "an"],

	FORMAT_KEY_SEQUENCE: [
		"@strip-periods",
		"@font-style",
		"@font-variant",
		"@font-weight",
		"@text-decoration",
		"@vertical-align",
		"@quotes"
	],

	INSTITUTION_KEYS: [
		"font-style",
		"font-variant",
		"font-weight",
		"text-decoration"
	],

	SUFFIX_CHARS: "a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z",
	ROMAN_NUMERALS: [
		[ "", "i", "ii", "iii", "iv", "v", "vi", "vii", "viii", "ix" ],
		[ "", "x", "xx", "xxx", "xl", "l", "lx", "lxx", "lxxx", "xc" ],
		[ "", "c", "cc", "ccc", "cd", "d", "dc", "dcc", "dccc", "cm" ],
		[ "", "m", "mm", "mmm", "mmmm", "mmmmm"]
	],
	CREATORS: [
		"author",
		"editor",
		"contributor",
		"translator",
		"recipient",
		"interviewer",
		"composer",
		"original-author",
		"container-author",
		"collection-editor"
	],

	LANG_BASES: {
		af: "af_ZA",
		ar: "ar_AR",
		bg: "bg_BG",
		ca: "ca_AD",
		cs: "cs_CZ",
		da: "da_DK",
		de: "de_DE",
		el: "el_GR",
		en: "en_US",
		es: "es_ES",
		et: "et_EE",
		fr: "fr_FR",
		he: "he_IL",
		hu: "hu_HU",
		is: "is_IS",
		it: "it_IT",
		ja: "ja_JP",
		km: "km_KH",
		ko: "ko_KR",
		mn: "mn_MN",
		nb: "nb_NO",
		nl: "nl_NL",
		pl: "pl_PL",
		pt: "pt_PT",
		ro: "ro_RO",
		ru: "ru_RU",
		sk: "sk_SK",
		sl: "sl_SI",
		sr: "sr_RS",
		sv: "sv_SE",
		th: "th_TH",
		tr: "tr_TR",
		uk: "uk_UA",
		vi: "vi_VN",
		zh: "zh_CN"
	},

	locale: {},
	locale_opts: {},
	locale_dates: {}

};

CSL.TERMINAL_PUNCTUATION_REGEXP = new RegExp("^([" + CSL.TERMINAL_PUNCTUATION.slice(0, -1).join("") + "])(.*)");
CSL.CLOSURES = new RegExp(".*[\\]\\)]");


//SNIP-START

// skip jslint check on this file, it doesn't get E4X
if (!CSL.debug) {
	load("./src/print.js");
}
if (!CSL.System) {
	load("./src/system.js");
}
if (!CSL.System.Xml.E4X) {
	load("./src/xmle4x.js");
}
if (!CSL.System.Xml.DOM) {
	load("./src/xmldom.js");
}
// jslint OK
if (!CSL.Mode) {
	load("./src/util_processor.js");
}
// jslint OK
if (!CSL.cloneAmbigConfig) {
	load("./src/util_disambig.js");
}
// jslint OK
if (!CSL.XmlToToken) {
	load("./src/util_nodes.js");
}
// jslint OK
if (!CSL.DateParser) {
	load("./src/util_dateparser.js");
}
// jslint OK
if (!CSL.Engine) {
	load("./src/build.js");
}
if (!CSL.Engine.prototype.setOutputFormat) {
	load("./src/api_control.js");
}

// jslint OK
if (!CSL.Output) {
	load("./src/queue.js");
}
// jslint OK
if (!CSL.Engine.Opt) {
	load("./src/state.js");
}
// jslint OK
if (!CSL.makeCitationCluster) {
	load("./src/api_cite.js");
}
// jslint OK
if (!CSL.makeBibliography) {
	load("./src/api_bibliography.js");
}
// jslint OK
if (!CSL.setCitationId) {
	load("./src/util_integration.js");
}
// jslint OK
if (!CSL.updateItems) {
	load("./src/api_update.js");
}
if (!CSL.localeResolve) {
	load("./src/util_locale.js");
}
if (!CSL.Node) {
	// jslint OK
	load("./src/node_bibliography.js");
	// jslint OK
    load("./src/node_choose.js");
	// jslint OK
    load("./src/node_citation.js");
    load("./src/node_comment.js");
	// jslint OK
	// jslint OK
    load("./src/node_date.js");
	// jslint OK
    load("./src/node_datepart.js");
	// jslint OK
    load("./src/node_elseif.js");
	// jslint OK
    load("./src/node_else.js");
	// jslint OK
    load("./src/node_etal.js");
	// jslint OK
    load("./src/node_group.js");
	// jslint OK
    load("./src/node_if.js");
	// jslint OK
    load("./src/node_info.js");
	// jslint OK
    load("./src/node_institution.js");
	// jslint OK
    load("./src/node_institutionpart.js");
	// jslint OK
    load("./src/node_key.js");
	// jslint OK
    load("./src/node_label.js");
	// jslint OK
    load("./src/node_layout.js");
	// jslint OK
    load("./src/node_macro.js");
	// jslint OK
    load("./src/node_name.js");
	// jslint OK
    load("./src/node_namepart.js");
	// jslint OK
    load("./src/node_names.js");
	// jslint OK
    load("./src/node_number.js");
	// jslint OK
    load("./src/node_sort.js");
	// jslint OK
    load("./src/node_substitute.js");
	// jslint OK
    load("./src/node_text.js");
}
// jslint OK
if (!CSL.Attributes) {
	load("./src/attributes.js");
}
// jslint OK
if (!CSL.Stack) {
	load("./src/stack.js");
}
// jslint OK
if (!CSL.Parallel) {
	load("./src/util_parallel.js");
}
// jslint OK
if (!CSL.Util) {
	load("./src/util.js");
}
// jslint OK
if (!CSL.Transform) {
	load("./src/util_transform.js");
}
// jslint OK
if (!CSL.Token) {
	load("./src/obj_token.js");
}
// jslint OK
if (!CSL.AmbigConfig) {
	load("./src/obj_ambigconfig.js");
}
// jslint OK
if (!CSL.Blob) {
	load("./src/obj_blob.js");
}
if (!CSL.Render) {
	load("./src/render.js");
}
// jslint OK
if (!CSL.NumericBlob) {
	load("./src/obj_number.js");
}
// jslint OK
if (!CSL.Util.fixDateNode) {
	load("./src/util_datenode.js");
}
// jslint OK
if (!CSL.Util.Names) {
	load("./src/util_names.js");
}
// jslint OK
if (!CSL.Util.Institutions) {
	load("./src/util_institutions.js");
}
// jslint OK (jslint wants "long" and "short" properties set in dot
// notation, but these are reserved words in JS, and raise an error
// in rhino.  Setting them in brace notation avoids the processing error.)
if (!CSL.Util.Dates) {
	load("./src/util_dates.js");
}
// jslint OK
if (!CSL.Util.Sort) {
	load("./src/util_sort.js");
}
// jslint OK
if (!CSL.Util.substituteStart) {
	load("./src/util_substitute.js");
}
// jslint OK
if (!CSL.Util.Suffixator) {
	load("./src/util_number.js");
}
// jstlint OK
if (!CSL.Util.PageRangeMangler) {
	load("./src/util_page.js");
}
// jslint OK
if (!CSL.Util.FlipFlopper) {
	load("./src/util_flipflop.js");
}
// jslint OK
if (!CSL.Output.Formatters) {
	load("./src/formatters.js");
}
// jslint OK
if (!CSL.Output.Formats) {
	load("./src/formats.js");
}
// jslint OK
if (!CSL.Registry) {
	load("./src/registry.js");
}
// jslint OK
if (!CSL.Registry.NameReg) {
	load("./src/disambig_names.js");
}
// jslint OK
if (!CSL.Registry.CitationReg) {
	load("./src/disambig_citations.js");
}
// jslint OK
if (!CSL.Registry.prototype.disambiguateCites) {
	load("./src/disambig_cites.js");
}

//SNIP-END
