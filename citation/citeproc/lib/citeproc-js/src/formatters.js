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
 * A bundle of handy functions for text processing.
 * <p>Several of these are ripped off from various
 * locations in the Zotero source code.</p>
 * @namespace Toolkit of string functions
 */
CSL.Output.Formatters = {};


CSL.Output.Formatters.strip_periods = function (state, string) {
    return string.replace(/\./g, " ").replace(/\s*$/g, "").replace(/\s+/g, " ");
};


/**
 * A noop that just delivers the string.
 */
CSL.Output.Formatters.passthrough = function (state, string) {
	return string;
};

/**
 * Force all letters in the string to lowercase.
 */
CSL.Output.Formatters.lowercase = function (state, string) {
	var str = CSL.Output.Formatters.doppelString(string, CSL.TAG_USEALL);
	str.string = str.string.toLowerCase();
	return CSL.Output.Formatters.undoppelString(str);
};


/**
 * Force all letters in the string to uppercase.
 */
CSL.Output.Formatters.uppercase = function (state, string) {
	var str = CSL.Output.Formatters.doppelString(string, CSL.TAG_USEALL);
	str.string = str.string.toUpperCase();
	return CSL.Output.Formatters.undoppelString(str);
};


/**
 * Force capitalization of the first letter in the string, leave
 * the rest of the characters untouched.
 */
CSL.Output.Formatters["capitalize-first"] = function (state, string) {
	var str = CSL.Output.Formatters.doppelString(string, CSL.TAG_ESCAPE);
	if (str.string.length) {
		str.string = str.string.slice(0, 1).toUpperCase() + str.string.substr(1);
		return CSL.Output.Formatters.undoppelString(str);
	} else {
		return "";
	}
};


/**
 * Similar to <b>capitalize_first</b>, but force the
 * subsequent characters to lowercase.
 */
CSL.Output.Formatters.sentence = function (state, string) {
	var str = CSL.Output.Formatters.doppelString(string, CSL.TAG_ESCAPE);
	str.string = str.string.slice(0, 1).toUpperCase() + str.string.substr(1).toLowerCase();
	return CSL.Output.Formatters.undoppelString(str);
};


/**
 * Force the first letter of each space-delimited
 * word in the string to uppercase, and force remaining
 * letters to lowercase.  Single characters are forced
 * to uppercase.
 */
CSL.Output.Formatters["capitalize-all"] = function (state, string) {
	var str, strings, len, pos;
	str = CSL.Output.Formatters.doppelString(string, CSL.TAG_ESCAPE);
	strings = str.string.split(" ");
	len = strings.length;
	for (pos = 0; pos < len; pos += 1) {
		if (strings[pos].length > 1) {
            strings[pos] = strings[pos].slice(0, 1).toUpperCase() + strings[pos].substr(1).toLowerCase();
        } else if (strings[pos].length === 1) {
            strings[pos] = strings[pos].toUpperCase();
        }
    }
	str.string = strings.join(" ");
	return CSL.Output.Formatters.undoppelString(str);
};

/**
 * A complex function that attempts to produce a pattern
 * of capitalization appropriate for use in a title.
 * Will not touch words that have some capitalization
 * already.
 */
CSL.Output.Formatters.title = function (state, string) {
	var str, words, isUpperCase, newString, lastWordIndex, previousWordIndex, upperCaseVariant, lowerCaseVariant, pos, skip, notfirst, notlast, firstword, aftercolon, len, idx, tmp, skipword, ppos, mx, lst, myret;
	str = CSL.Output.Formatters.doppelString(string, CSL.TAG_ESCAPE);
	if (!string) {
		return "";
	}

	// split words
	// Workaround for Internet Explorer
	mx = str.string.match(/(\s+)/g);
	lst = str.string.split(/\s+/);
	myret = [lst[0]];
	for (pos = 1, len = lst.length; pos < len; pos += 1) {
		myret.push(mx[pos - 1]);
		myret.push(lst[pos]);
	}
	words = myret.slice();
	isUpperCase = str.string.toUpperCase() === string;
	newString = "";
	lastWordIndex = words.length - 1;
	previousWordIndex = -1;
	for (pos = 0; pos <= lastWordIndex;  pos += 2) {
		if (words[pos].length !== 0 && (words[pos].length !== 1 || !/\s+/.test(words[pos]))) {
			upperCaseVariant = words[pos].toUpperCase();
			lowerCaseVariant = words[pos].toLowerCase();
			if (isUpperCase || words[pos] === lowerCaseVariant) {
				skip = false;
				len = CSL.SKIP_WORDS.length;
				for (ppos = 0; ppos < len; ppos += 1) {
					skipword = CSL.SKIP_WORDS[ppos];
					idx = lowerCaseVariant.indexOf(skipword);
					if (idx > -1) {
						tmp = lowerCaseVariant.slice(0, idx, idx + lowerCaseVariant.slice(skipword.length));
						if (!tmp.match(/[a-zA-Z]/)) {
							skip = true;
						}
					}
				}
				notfirst = pos !== 0;
				notlast = pos !== lastWordIndex;
				if (words[previousWordIndex]) {
					aftercolon = words[previousWordIndex].slice(-1) !== ":";
				} else {
					aftercolon = false;
				}
				if (skip && notfirst && notlast && (firstword || aftercolon)) {
					words[pos] = lowerCaseVariant;
				} else {
					words[pos] = upperCaseVariant.slice(0, 1) + lowerCaseVariant.substr(1);
				}
			}
			previousWordIndex = pos;
		}
	}
	str.string = words.join("");
	return CSL.Output.Formatters.undoppelString(str);
};

CSL.Output.Formatters.doppelString = function (string, rex) {
	var ret, pos, len;
	ret = {};
	// rex is a function that returns an appropriate array.
	//
	// XXXXX: Does this work in Internet Explorer?
	//
	ret.array = rex(string);
	// ret.array = string.split(rex);
	ret.string = "";
	len = ret.array.length;
	for (pos = 0; pos < len; pos += 2) {
		ret.string += ret.array[pos];
	}
	return ret;
};


CSL.Output.Formatters.undoppelString = function (str) {
	var ret, len, pos;
	ret = "";
	len = str.array.length;
	for (pos = 0; pos < len; pos += 1) {
		if ((pos % 2)) {
			ret += str.array[pos];
		} else {
			ret += str.string.slice(0, str.array[pos].length);
			str.string = str.string.slice(str.array[pos].length);
		}
	}
	return ret;
};


CSL.Output.Formatters.serializeItemAsRdf = function (Item) {
	return "";
};


CSL.Output.Formatters.serializeItemAsRdfA = function (Item) {
	return "";
};
