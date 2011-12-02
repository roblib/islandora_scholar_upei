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

CSL.Util.Names = {};

/**
 * Build a set of names, less any label or et al. tag
 */
CSL.Util.Names.outputNames = function (state, display_names) {
	var segments, and;
	segments = new this.StartMiddleEnd(state, display_names);
	and = state.output.getToken("name").strings.delimiter;
	if (state.tmp.use_ellipsis) {
	    //		and = state.output.getToken("inner").strings.delimiter + state.getTerm("ellipsis") + " ";
		and = state.output.getToken("inner").strings.delimiter + "\u2026 ";
	} else if (state.output.getToken("name").strings["delimiter-precedes-last"] === "always") {
		and = state.output.getToken("inner").strings.delimiter + and;
	} else if (state.output.getToken("name").strings["delimiter-precedes-last"] === "never") {
		if (!and) {
			and = state.output.getToken("inner").strings.delimiter;
		}
	} else if ((segments.segments.start.length + segments.segments.middle.length) > 1) {
		and = state.output.getToken("inner").strings.delimiter + and;
	} else {
		if (!and) {
			and = state.output.getToken("inner").strings.delimiter;
		}
	}
	if (and.match(CSL.STARTSWITH_ROMANESQUE_REGEXP)) {
		and = " " + and;
	}
	if (and.match(CSL.ENDSWITH_ROMANESQUE_REGEXP)) {
		and = and + " ";
	}
	state.output.getToken("name").strings.delimiter = and;

	state.output.openLevel("name");
	state.output.openLevel("inner");
	segments.outputSegmentNames("start");
	segments.outputSegmentNames("middle");
	state.output.closeLevel(); // inner
	segments.outputSegmentNames("end");
	state.output.closeLevel(); // name
};

CSL.Util.Names.StartMiddleEnd = function (state, names) {
	var start, middle, endstart, end, ret;
	this.state = state;
	this.nameoffset = 0;
	//
	// what to do here?  we need config for this, tokens to
	// control the joining that will come.  how do we get
	// them into this function?
	start = names.slice(0, 1);
	middle = names.slice(1, (names.length - 1));
	endstart = 1;
	if (names.length > 1) {
		endstart = (names.length - 1);
	}
	end = names.slice(endstart, (names.length));
	ret = {};
	ret.start = start;
	ret.middle = middle;
	ret.end = end;
	this.segments = ret;
};

CSL.Util.Names.StartMiddleEnd.prototype.outputSegmentNames = function (seg) {
	var state, value, sequence, pos, len;
	state = this.state;
	len = this.segments[seg].length;
	for (pos = 0; pos < len; pos += 1) {
		this.namenum = parseInt(pos, 10);
		this.name = this.segments[seg][pos];
		// Get the language tags from the names transliteration
		// preference, and feed it to the following function.
		var translit = state.opt["locale-pri"];
		this.outputName(seg, pos, translit);
	}
	this.nameoffset += this.segments[seg].length;
};

CSL.Util.Names.StartMiddleEnd.prototype.outputName = function (seg, pos, translit, tokenname) {

	var name = this.state.transform.name(this.state, this.name, translit);

	if (name.literal) {
		value = name.literal;
		this.state.output.append(name.literal, "empty");
	} else {

		if (name.transliterated) {
			this.state.output.openLevel("empty");
		}

		if (tokenname) {
			this.state.output.openLevel(tokenname);
		} 

		sequence = CSL.Util.Names.getNamepartSequence(this.state, seg, name);
		
		this.state.output.openLevel(sequence[0][0]); // articular join
		this.state.output.openLevel(sequence[0][1]); // join to last element (?)
		this.state.output.openLevel(sequence[0][2]); // inter-element join (?)
		
		this.outputNameParts(name, sequence[1]);
		
		this.state.output.closeLevel();
		this.state.output.openLevel(sequence[0][2]);
		
		this.outputNameParts(name, sequence[2]);
		
		this.state.output.closeLevel();
		this.state.output.closeLevel();
		//
		// articular goes here  //
		//
		this.outputNameParts(name, sequence[3]);
		
		this.state.output.closeLevel();

		if (tokenname) {
			this.state.output.closeLevel(); // parens
		}

		if (this.state.opt["locale-show-original-names"] 
			&& this.state.tmp.area === "bibliography"
			&& name.transliterated 
			&& this.name.given) {

			var parens = new CSL.Blob();
			parens.strings.prefix = " (";
			parens.strings.suffix = ")";
			this.state.output.addToken("parens", false, parens);
			this.outputName(seg, pos, false, "parens");
		}

		if (name.transliterated) {
			this.state.output.closeLevel(); // wrapper to avoid delimiter between names.
		}

	}
	return name.transliterated;
};

CSL.Util.Names.StartMiddleEnd.prototype.outputNameParts = function (name, subsequence) {
	var state, len, pos, key, namepart, initialize_with, preffie;
	state = this.state;
    	// Purge empty name parts from keylist
	for (var i = subsequence.length - 1; i > -1; i += -1) {
	    if (!name[subsequence[i]]) {
		subsequence = subsequence.slice(0, i).concat(subsequence.slice(i + 1));
	    }
	}
	preffie = "";
	len = subsequence.length;
	for (pos = 0; pos < len; pos += 1) {
		key = subsequence[pos];
		namepart = name[key];
		if (preffie) {
		    namepart = preffie + namepart;
		    preffie = "";
		}
		// Do not include given name, dropping particle or suffix in strict short form of name
		if (["given", "suffix", "dropping-particle"].indexOf(key) > -1 && 0 === state.tmp.disambig_settings.givens[state.tmp.nameset_counter][this.namenum + this.nameoffset]) {
			if (!(key === "given" && !name.family)) {
				if (key === "suffix") {
					if (name.suffix !== name.suffix.toLowerCase()) {
						continue;
					}
				} else {
					continue;
				}
			}
		}
		// If ends in an apostrophe, is a particle, and is immediately
		// followed by family, merge particle to family.
		if (key === "dropping-particle" 
		    && ["'","\u02bc","\u2019"].indexOf(namepart.slice(-1)) > -1
		    && pos < subsequence.length - 1
		    && subsequence[pos + 1] === "family") {
			preffie = namepart;
			continue;
		}
		// initialize if appropriate
		if ("given" === key) {
			if (1 === state.tmp.disambig_settings.givens[state.tmp.nameset_counter][(this.namenum + this.nameoffset)] && !name.block_initialize) {
				initialize_with = state.output.getToken("name").strings["initialize-with"];
				namepart = CSL.Util.Names.initializeWith(state, namepart, initialize_with);
			} else {
				namepart = CSL.Util.Names.unInitialize(state, namepart);
			}
		}
		state.output.append(namepart, key);
	}
};

CSL.Util.Names.getNamepartSequence = function (state, seg, name) {
	var token, suffix_sep, romanesque, sequence;
	token = state.output.getToken("name");
	// Set the rendering order and separators of the core nameparts
	// sequence[0][0] separates elements inside each of the the two lists
	// sequence[0][1] separates the two lists
	if (name["comma-suffix"]) {
		state.output.getToken("suffixsep").strings.delimiter = ", ";
	} else {
		state.output.getToken("suffixsep").strings.delimiter = " ";
	}
	romanesque = name.family.match(CSL.ROMANESQUE_REGEXP);
	// neither roman nor Cyrillic characters
	if (!romanesque) {
		sequence = [["empty", "empty", "empty"], ["non-dropping-particle", "family"], ["given"], []];
	} else if (name["static-ordering"]) { // entry likes sort order
		sequence = [["empty", "space", "space"], ["non-dropping-particle", "family"], ["given"], []];
	} else if (state.tmp.sort_key_flag) {
		if (state.opt["demote-non-dropping-particle"] === "never") {
			sequence = [["suffixsep", "sortsep", "space"], ["non-dropping-particle", "family", "dropping-particle"], ["given"], ["suffix"]];
		} else {
			sequence = [["suffixsep", "sortsep", "space"], ["family"], ["given", "dropping-particle", "non-dropping-particle"], ["suffix"]];
		}
	} else if (token && (token.strings["name-as-sort-order"] === "all" || (token.strings["name-as-sort-order"] === "first" && seg === "start"))) {
		//
		// Discretionary sort ordering and inversions
		//
		if (["always", "display-and-sort"].indexOf(state.opt["demote-non-dropping-particle"]) > -1) {
			// Drop non-dropping particle
			sequence = [["sortsep", "sortsep", "space"], ["family"], ["given", "dropping-particle", "non-dropping-particle"], ["suffix"]];
		} else {
			// Don't drop particle.
			sequence = [["sortsep", "sortsep", "space"], ["non-dropping-particle", "family"], ["given", "dropping-particle"], ["suffix"]];
		}
	} else { // plain vanilla
		sequence = [["suffixsep", "space", "space"], ["given"], ["dropping-particle", "non-dropping-particle", "family"], ["suffix"]];
	}
	return sequence;
};

//
// Apparently never used.
//
//CSL.Util.Names.deep_copy = function (nameset) {
//	var nameset2, len, pos, name2, name;
//	print("USING");
//	nameset2 = [];
//	len = nameset.length;
//	for (pos = 0; pos < len; pos += 1) {
//		name = nameset[pos];
//		name2 = {};
//		for (var i in name) {
//			name2[i] = name[i];
//		}
//		nameset2.push(name2);
//	}
//	return nameset2;
//};


/**
 * Reinitialize scratch variables used by names machinery.
 */
//
// XXXX A handy guide to variable assignments that need
// XXXX to be eliminated.  :)
//
CSL.Util.Names.reinit = function (state, Item) {
	state.tmp.value = [];
	state.tmp.name_et_al_term = false;
	state.tmp.name_et_al_decorations = false;


	state.tmp.name_et_al_form = "long";
	state.tmp.et_al_prefix = false;
};

CSL.Util.Names.getCommonTerm = function (state, namesets) {
	var base_nameset, varnames, len, pos, short_namesets, nameset;
	if (namesets.length < 2) {
		return false;
	}
	base_nameset = namesets[0];
	
	varnames = [];
	varnames.push(base_nameset.variable);
	short_namesets = namesets.slice(1);
	len = short_namesets.length;
	for (pos = 0; pos < len; pos += 1) {
		nameset = short_namesets[pos];
		if (!CSL.Util.Names.compareNamesets(base_nameset, nameset)) {
			return false;
		}
		if (varnames.indexOf(nameset.variable) === -1) {
			varnames.push(nameset.variable);
		}
	}
	varnames.sort();
	var combined_terms = varnames.join("");
	if (state.locale[state.opt.lang].terms[combined_terms]) {
		return varnames.join("");
	} else {
		return false;
	}
};


CSL.Util.Names.compareNamesets = function (base_nameset, nameset) {
	var name, pos, len, part, ppos, llen;
	if (!base_nameset.names || !nameset.names || base_nameset.names.length !== nameset.names.length || base_nameset.etal !== nameset.etal) {
		return false;
	}
	len = nameset.names.length;
	for (pos = 0; pos < len; pos += 1) {
		name = nameset.names[pos];
		llen = CSL.NAME_PARTS.length;
		for (ppos = 0; ppos < llen; ppos += 1) {
			part = CSL.NAME_PARTS[ppos];
			if (!base_nameset.names[pos] || base_nameset.names[pos][part] != name[part]) {
				return false;
			}
		}
	}
	return true;
};

/**
 * Un-initialize a name (quash caps after first character)
 */
CSL.Util.Names.unInitialize = function (state, name) {
	var namelist, punctlist, ret, pos, len;
	if (!name) {
		return "";
	}
	namelist = name.split(/(?:\-|\s+)/);
	punctlist = name.match(/(\-|\s+)/g);
	ret = "";
	for (pos = 0, len = namelist.length; pos < len; pos += 1) {
		if (CSL.ALL_ROMANESQUE_REGEXP.exec(namelist[pos].slice(0,-1))) {
			namelist[pos] = namelist[pos].slice(0, 1) + namelist[pos].slice(1).toLowerCase();
		}
		ret += namelist[pos];
		if (pos < len - 1) {
			ret += punctlist[pos];
		}
	}
	return ret;
};

/**
 * Initialize a name.
 */
CSL.Util.Names.initializeWith = function (state, name, terminator) {
	var namelist, l, i, n, m, extra, ret, s, c, pos, len, ppos, llen, llst, mx, lst;
	if (!name) {
		return "";
	}
	if (!terminator) {
		terminator = "";
	}
	namelist = name;
	if (state.opt["initialize-with-hyphen"] === false) {
		namelist = namelist.replace(/\-/g, " ");
	}
	namelist = namelist.replace(/\./g, " ").replace(/\s*\-\s*/g, "-").replace(/\s+/g, " ");
	// Workaround for Internet Explorer
	namelist = namelist.split(/(\-|\s+)/);
	for (i = 0, ilen = namelist.length; i < ilen; i += 2) {
		n = namelist[i];
		if (!n) {
			continue;
		}
		m = n.match(CSL.NAME_INITIAL_REGEXP);
		if (!m && (!n.match(CSL.STARTSWITH_ROMANESQUE_REGEXP) && n.length > 1 && terminator.match("%s"))) {
			m = n.match(/(.)(.*)/);
		}
		if (m && m[1] === m[1].toUpperCase()) {
			extra = "";
			if (m[2]) {
				s = "";
				lst = m[2].split("");
				for (j = 0, jlen = lst.length; j < jlen; j += 1) {
					c = lst[j];
					if (c === c.toUpperCase()) {
						s += c;
					} else {
						break;
					}
				}
				if (s.length < m[2].length) {
					extra = s.toLocaleLowerCase();
				}
			}
			namelist[i] = m[1].toLocaleUpperCase() + extra;
			if (i < (ilen - 1)) {
				if (terminator.match("%s")) {
					namelist[i] = terminator.replace("%s", namelist[i]);
				} else {
					if (namelist[i + 1].indexOf("-") > -1) {
						namelist[i + 1] = terminator + namelist[i + 1];
					} else {
						namelist[i + 1] = terminator;
					}
				}
			} else {
				if (terminator.match("%s")) {
					namelist[i] = terminator.replace("%s", namelist[i]);
				} else {
					namelist.push(terminator);
				}
			}
		} else if (n.match(CSL.ROMANESQUE_REGEXP)) {
			namelist[i] = " " + n;
		}
	}
	ret = CSL.Util.Names.stripRight(namelist.join(""));
	ret = ret.replace(/\s*\-\s*/g, "-").replace(/\s+/g, " ");
	return ret;
};


CSL.Util.Names.stripRight = function (str) {
	var end, pos, len;
	end = 0;
	len = str.length - 1;
	for (pos = len; pos > -1; pos += -1) {
		if (str[pos] !== " ") {
			end = pos + 1;
			break;
		}
	}
	return str.slice(0, end);
};

CSL.Util.Names.initNameSlices = function (state) {
	var len, pos;
	state.tmp.names_cut = {
		counts: [],
		variable: {}
	};
	len = CSL.NAME_VARIABLES.length;
	for (pos = 0; pos < len; pos += 1) {
		state.tmp.names_cut.counts[CSL.NAME_VARIABLES[pos]] = 0;
	}
};

// deleted CSL.Util.Names,rescueNameElements()
// apparently not used.


CSL.Engine.prototype.parseName = function (name) {
	var m, idx;
	if (! name["non-dropping-particle"] && name.family) {
		m = name.family.match(/^([ a-z]+\s+)/);
		if (m) {
			name.family = name.family.slice(m[1].length);
			name["non-dropping-particle"] = m[1].replace(/\s+$/, "");

		}
	}
	if (!name.suffix && name.given) {
		m = name.given.match(/(\s*,!*\s*)/);
		if (m) {
			idx = name.given.indexOf(m[1]);
			if (name.given.slice(idx, idx + m[1].length).replace(/\s*/g, "").length === 2) {
				name["comma-suffix"] = true;
			}
			name.suffix = name.given.slice(idx + m[1].length);
			name.given = name.given.slice(0, idx);
		}
	}
	if (! name["dropping-particle"] && name.given) {
		m = name.given.match(/^(\s+[ a-z]*[a-z])$/);
		if (m) {
			name.given = name.given.slice(0, m[1].length * -1);
			name["dropping-particle"] = m[2].replace(/^\s+/, "");
		}
	}
};
