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

CSL.Output = {};
/**
 * Output queue object.
 * @class
 */
CSL.Output.Queue = function (state) {
	this.levelname = ["top"];
	this.state = state;
	this.queue = [];
	this.empty = new CSL.Token("empty");
	var tokenstore = {};
	tokenstore.empty = this.empty;
	this.formats = new CSL.Stack(tokenstore);
	this.current = new CSL.Stack(this.queue);
};

CSL.Output.Queue.prototype.getToken = function (name) {
	var ret = this.formats.value()[name];
	return ret;
};

CSL.Output.Queue.prototype.mergeTokenStrings = function (base, modifier) {
	var base_token, modifier_token, ret, key;
	base_token = this.formats.value()[base];
	modifier_token = this.formats.value()[modifier];
	ret = base_token;
	if (modifier_token) {
		if (!base_token) {
			base_token = new CSL.Token(base, CSL.SINGLETON);
			base_token.decorations = [];
		}
		ret = new CSL.Token(base, CSL.SINGLETON);
		key = "";
		for (key in base_token.strings) {
			if (base_token.strings.hasOwnProperty(key)) {
				ret.strings[key] = base_token.strings[key];
			}
		}
		for (key in modifier_token.strings) {
			if (modifier_token.strings.hasOwnProperty(key)) {
				ret.strings[key] = modifier_token.strings[key];
			}
		}
		ret.decorations = base_token.decorations.concat(modifier_token.decorations);
	}
	return ret;
};

// Store a new output format token based on another
CSL.Output.Queue.prototype.addToken = function (name, modifier, token) {
	var newtok, attr;
	newtok = new CSL.Token("output");
	if ("string" === typeof token) {
		token = this.formats.value()[token];
	}
	if (token && token.strings) {
		for (attr in token.strings) {
			if (token.strings.hasOwnProperty(attr)) {
				newtok.strings[attr] = token.strings[attr];
			}
		}
		newtok.decorations = token.decorations;

	}
	if ("string" === typeof modifier) {
		newtok.strings.delimiter = modifier;
	}
	this.formats.value()[name] = newtok;
};

//
// newFormat adds a new bundle of formatting tokens to
// the queue's internal stack of such bundles
CSL.Output.Queue.prototype.pushFormats = function (tokenstore) {
	if (!tokenstore) {
		tokenstore = {};
	}
	tokenstore.empty = this.empty;
	this.formats.push(tokenstore);
};


CSL.Output.Queue.prototype.popFormats = function (tokenstore) {
	this.formats.pop();
};

CSL.Output.Queue.prototype.startTag = function (name, token) {
	var tokenstore = {};
	tokenstore[name] = token;
	this.pushFormats(tokenstore);
	this.openLevel(name);
};

CSL.Output.Queue.prototype.endTag = function () {
	this.closeLevel();
	this.popFormats();
};

//
// newlevel adds a new blob object to the end of the current
// list, and adjusts the current pointer so that subsequent
// appends are made to blob list of the new object.

CSL.Output.Queue.prototype.openLevel = function (token, ephemeral) {
	var blob, curr, x, has_ephemeral;
	if (!this.formats.value()[token]) {
		throw "CSL processor error: call to nonexistent format token \"" + token + "\"";
	}
	// delimiter, prefix, suffix, decorations from token
	blob = new CSL.Blob(this.formats.value()[token], false, token);
	if (this.state.tmp.count_offset_characters && blob.strings.prefix.length) {
		this.state.tmp.offset_characters += blob.strings.prefix.length;
	}
	if (this.state.tmp.count_offset_characters && blob.strings.suffix.length) {
		this.state.tmp.offset_characters += blob.strings.suffix.length;
	}
	curr = this.current.value();
	has_ephemeral = false;
	for (x in this.state.tmp.names_cut.variable) {
		if (this.state.tmp.names_cut.variable.hasOwnProperty(x)) {
			has_ephemeral = x;
			break;
		}
	}
	// can only do this for one variable
	if (ephemeral && (!has_ephemeral || ephemeral === has_ephemeral)) {
		if (!this.state.tmp.names_cut.variable[ephemeral]) {
			this.state.tmp.names_cut.variable[ephemeral] = [];
			this.state.tmp.names_cut.used = ephemeral;
		}
		this.state.tmp.names_cut.variable[ephemeral].push([curr, curr.blobs.length]);
	}
	curr.push(blob);
	this.current.push(blob);
};

/**
 * "merge" used to be real complicated, now it's real simple.
 */
CSL.Output.Queue.prototype.closeLevel = function (name) {
	// CLEANUP: Okay, so this.current.value() holds the blob at the
	// end of the current list.  This is wrong.  It should
	// be the parent, so that we have  the choice of reading
	// the affixes and decorations, or appending to its
	// content.  The code that manipulates blobs will be
	// much simpler that way.
	if (name && name !== this.current.value().levelname) {
		CSL.error("Level mismatch error:  wanted " + name + " but found " + this.current.value().levelname);
	}
	this.current.pop();
};

//
// append does the same thing as newlevel, except
// that the blob it pushes has text content,
// and the current pointer is not moved after the push.

CSL.Output.Queue.prototype.append = function (str, tokname) {
	var token, blob, curr;
	if ("undefined" === typeof str) {
		return;
	}
	if ("number" === typeof str) {
		str = "" + str;
	}
	if (this.state.tmp.element_trace && this.state.tmp.element_trace.value() === "suppress-me") {
		return;
	}
	blob = false;
	if (!tokname) {
		token = this.formats.value().empty;
	} else if (tokname === "literal") {
		token = true;
	} else if ("string" === typeof tokname) {
		token = this.formats.value()[tokname];
	} else {
		token = tokname;
	}
	if (!token) {
		throw "CSL processor error: unknown format token name: " + tokname;
	}
	if ("string" === typeof str && str.length) {
		this.last_char_rendered = str.slice(-1);
		// This, and not the str argument below on flipflop, is the
		// source of the flipflopper string source.
		str = str.replace(/\s+'/g, "  \'").replace(/^'/g, " \'");
		// signal whether we end with terminal punctuation?
		this.state.tmp.term_predecessor = true;
	}
	blob = new CSL.Blob(token, str);
	if (this.state.tmp.count_offset_characters && blob.strings.prefix) {
		this.state.tmp.offset_characters += blob.strings.prefix.length;
	}
	if (this.state.tmp.count_offset_characters && blob.strings.suffix) {
		this.state.tmp.offset_characters += blob.strings.suffix.length;
	}
	curr = this.current.value();
	if ("string" === typeof blob.blobs) {
		this.state.tmp.term_predecessor = true;
	}
	//
	// XXXXX: Interface to this function needs cleaning up.
	// The str variable is ignored if blob is given, and blob
	// must contain the string to be processed.  Ugly.
	//CSL.debug("str:"+str.length);
	//CSL.debug("blob:"+blob);
	//CSL.debug("tokname:"+tokname);
	//
	// <Dennis Hopper impersonation>
	// XXXXX: This is, like, too messed up for _words_, man.
	// </Dennis Hopper impersonation>
	//
	if (this.state.tmp.count_offset_characters) {
		if ("string" === typeof str) {
			//
			// XXXXX: for all this offset stuff, need to strip affixes
			// before measuring; they may contain markup tags.
			//
			this.state.tmp.offset_characters += blob.strings.prefix.length;
			this.state.tmp.offset_characters += blob.strings.suffix.length;
			this.state.tmp.offset_characters += blob.blobs.length;
		} else if ("undefined" !== str.num) {
			this.state.tmp.offset_characters += str.strings.prefix.length;
			this.state.tmp.offset_characters += str.strings.suffix.length;
			this.state.tmp.offset_characters += str.formatter.format(str.num).length;
		}
	}
	//
	// Caution: The parallel detection machinery will blow up if tracking
	// variables are not properly initialized elsewhere.
	//
	this.state.parallel.AppendBlobPointer(curr);
	if ("string" === typeof str) {
		curr.push(blob);
		if (blob.strings["text-case"]) {
			//
			// This one is _particularly_ hard to follow.  It's not obvious,
			// but the blob already contains the input string at this
			// point, as blob.blobs -- it's a terminal node, as it were.
			// The str variable also contains the input string, but
			// that copy is not used for onward processing.  We have to
			// apply our changes to the blob copy.
			//
			blob.blobs = CSL.Output.Formatters[blob.strings["text-case"]](this.state, str);
		}
		//
		// XXX: Beware superfluous code in your code.  str in this
		// case is not the source of the final rendered string.
		// See note above.
		//
		this.state.fun.flipflopper.init(str, blob);
		//CSL.debug("(queue.append blob decorations): "+blob.decorations);
		this.state.fun.flipflopper.processTags();
	} else {
		curr.push(str);
	}
};

//
// Maybe the way to do this is to take it by layers, and
// analyze a FLAT list of blobs returned during recursive
// execution.  If the list is all numbers and there is no
// group decor, don't touch it.  If it ends in numbers,
// set the group delimiter on the first in the series,
// and join the strings with the group delimiter.  If it
// has numbers followed by strings, render each number
// in place, and join with the group delimiter.  Return
// the mixed flat list, and recurse upward.
//
// That sort of cascade should work, and should be more
// easily comprehensible than this mess.
//

CSL.Output.Queue.prototype.string = function (state, myblobs, blob) {
	var blobs, ret, blob_delimiter, i, params, blobjr, last_str, last_char, b, use_suffix, qres, addtoret, span_split, j, res, blobs_start, blobs_end, key, pos, len, ppos, llen, ttype, ltype, terminal, leading, delimiters, use_prefix, txt_esc;
	txt_esc = CSL.Output.Formats[this.state.opt.mode].text_escape;
	blobs = myblobs.slice();
	ret = [];
	
	if (blobs.length === 0) {
		return ret;
	}

	if (!blob) {
		blob_delimiter = "";
	} else {
		blob_delimiter = blob.strings.delimiter;
	}

	if (blob && blob.new_locale) {
		state.opt.lang = blob.new_locale;
	}

	for (pos = 0, len = blobs.length; pos < len; pos += 1) {
		blobjr = blobs[pos];

		if ("string" === typeof blobjr.blobs) {

			if ("number" === typeof blobjr.num) {
				ret.push(blobjr);
			} else if (blobjr.blobs) {
				// (skips empty strings)
				b = blobjr.blobs;

				use_suffix = blobjr.strings.suffix;
				use_prefix = blobjr.strings.prefix;

				if (!state.tmp.suppress_decorations) {
					llen = blobjr.decorations.length;
					for (ppos = 0; ppos < llen; ppos += 1) {
						params = blobjr.decorations[ppos];
						b = state.fun.decorate[params[0]][params[1]](state, b);
					}
				}
				//
				// because we will rip out portions of the output
				// queue before rendering, group wrappers need
				// to produce no output if they are found to be
				// empty.
				if (b && b.length) {
					b = txt_esc(blobjr.strings.prefix) + b + txt_esc(use_suffix);
					ret.push(b);
				}
			}
		} else if (blobjr.blobs.length) {

			addtoret = state.output.string(state, blobjr.blobs, blobjr);
			if (ret.slice(-1)[0] && addtoret.slice(-1)[0]) {
				ttype = typeof ret.slice(-1)[0];
				ltype = typeof addtoret.slice(-1)[0];
				//
				// The list generated by the string function is a mixture
				// of strings and numeric data objects awaiting evaluation
				// for ranged joins.  If we hit one of them, we skip this
				// fixit operation.
				//
				if ("string" === ttype && "string" === ltype) {
					terminal = ret.slice(-1)[0].slice(-1);
					leading = addtoret.slice(-1)[0].slice(0, 1);
				}
			}
			ret = ret.concat(addtoret);
		} else {
			continue;
		}
	}
	span_split = 0;
	len = ret.length;
	for (pos = 0; pos < len; pos += 1) {
		if ("string" === typeof ret[pos]) {
			span_split = (parseInt(pos, 10) + 1);
		}
	}
	if (blob && (blob.decorations.length || blob.strings.suffix || blob.strings.prefix)) {
		span_split = ret.length;
	}
	blobs_start = state.output.renderBlobs(ret.slice(0, span_split), blob_delimiter);
	if (blobs_start && blob && (blob.decorations.length || blob.strings.suffix || blob.strings.prefix)) {
		if (!state.tmp.suppress_decorations) {
			len = blob.decorations.length;
			for (pos = 0; pos < len; pos += 1) {
				params = blob.decorations[pos];
				if (["@bibliography", "@display"].indexOf(params[0]) > -1) {
					continue;
				}
				blobs_start = state.fun.decorate[params[0]][params[1]](state, blobs_start);
			}
		}
		//
		// XXXX: cut-and-paste warning.  same as a code block above.
		//
		b = blobs_start;
		use_suffix = blob.strings.suffix;
		if (b && b.length) {
			use_prefix = blob.strings.prefix;
			b = txt_esc(use_prefix) + b + txt_esc(use_suffix);
		}
		blobs_start = b;
		if (!state.tmp.suppress_decorations) {
			len = blob.decorations.length;
			for (pos = 0; pos < len; pos += 1) {
				params = blob.decorations[pos];
				if (["@bibliography", "@display"].indexOf(params[0]) === -1) {
					continue;
				}
				blobs_start = state.fun.decorate[params[0]][params[1]](state, blobs_start);
			}
		}
	}
	blobs_end = ret.slice(span_split, ret.length);
	if (!blobs_end.length && blobs_start) {
		ret = [blobs_start];
	} else if (blobs_end.length && !blobs_start) {
		ret = blobs_end;
	} else if (blobs_start && blobs_end.length) {
		ret = [blobs_start].concat(blobs_end);
	}
	//
	// Blobs is now definitely a string with
	// trailing blobs.  Return it.
	if ("undefined" === typeof blob) {
		this.queue = [];
		this.current.mystack = [];
		this.current.mystack.push(this.queue);
		if (state.tmp.suppress_decorations) {
			ret = state.output.renderBlobs(ret);
		}
	} else if ("boolean" === typeof blob) {
		ret = state.output.renderBlobs(ret);
	}

	if (blob && blob.new_locale) {
		state.opt.lang = blob.old_locale;
	}

	if (blob) {
		return ret;
	} else {
		return ret;
	}
};

CSL.Output.Queue.prototype.clearlevel = function () {
	var blob, pos, len;
	blob = this.current.value();
	len = blob.blobs.length;
	for (pos = 0; pos < len; pos += 1) {
		blob.blobs.pop();
	}
};

CSL.Output.Queue.prototype.renderBlobs = function (blobs, delim) {
	var state, ret, ret_last_char, use_delim, i, blob, pos, len, ppos, llen, pppos, lllen, res, str, params, txt_esc;
	txt_esc = CSL.Output.Formats[this.state.opt.mode].text_escape;
	if (!delim) {
		delim = "";
	}
	state = this.state;
	ret = "";
	ret_last_char = [];
	use_delim = "";
	len = blobs.length;
	for (pos = 0; pos < len; pos += 1) {
		if (blobs[pos].checkNext) {
			blobs[pos].checkNext(blobs[(pos + 1)]);
		}
	}
	// Fix last non-range join
	var doit = true;
	for (pos = blobs.length - 1; pos > 0; pos += -1) {
	    if (blobs[pos].checkLast) {
		if (doit && blobs[pos].checkLast(blobs[pos - 1])) {
		    doit = false;
		}
	    } else {
		doit = true;
	    }
	}
	len = blobs.length;
	for (pos = 0; pos < len; pos += 1) {
		blob = blobs[pos];
		if (ret) {
			use_delim = delim;
		}
		if (blob && "string" === typeof blob) {
			ret += txt_esc(use_delim);
			ret += blob;
		} else if (blob.status !== CSL.SUPPRESS) {
			str = blob.formatter.format(blob.num, blob.gender);
			if (blob.strings["text-case"]) {
				str = CSL.Output.Formatters[blob.strings["text-case"]](this.state, str);
			}
			if (!state.tmp.suppress_decorations) {
				llen = blob.decorations.length;
				for (ppos = 0; ppos < llen; ppos += 1) {
					params = blob.decorations[ppos];
					str = state.fun.decorate[params[0]][params[1]](state, str);
				}
			}
			str = blob.strings.prefix + str + blob.strings.suffix;
			if (blob.status === CSL.END) {
				ret += blob.range_prefix;
			} else if (blob.status === CSL.SUCCESSOR) {
				ret += blob.successor_prefix;
			} else if (blob.status === CSL.START) {
				ret += "";
			} else if (blob.status === CSL.SEEN) {
				ret += blob.splice_prefix;
			}
			ret += str;
		}
	}
	return ret;
};

CSL.Output.Queue.purgeEmptyBlobs = function (myblobs, endOnly) {
	var res, j, jlen, tmpblobs;
	if ("string" === typeof myblobs || !myblobs.length) {
		return;
	}
	for (var i = myblobs.length - 1; i > -1; i += -1) {
		CSL.Output.Queue.purgeEmptyBlobs(myblobs[i].blobs);		
	}
	for (var i = myblobs.length - 1; i > -1; i += -1) {
		// Edit myblobs in place
		if (!myblobs[i].blobs.length) {
			tmpblobs = myblobs.slice(i + 1);
			for (j = i, jlen = myblobs.length; j < jlen; j += 1) {
				myblobs.pop();
			}
			for (j = 0, jlen = tmpblobs.length; j < jlen; j += 1) {
				myblobs.push(tmpblobs[j]);
			}
		} else if (endOnly) {
			break;
		}
	}
}

// XXXXX: Okay, stop and think about the following two functions.
// Spaces have no formatting characteristics, so they can be
// safely purged at lower levels.  If a separate function is used
// for punctuation (i.e. the original setup), and special-purpose
// functions are applied to spaces, we can get more robust 
// behavior without breaking things all over the place.

CSL.Output.Queue.purgeNearsidePrefixChars = function(myblob, chr) {
	if (!chr) {
		return;
	}
	if ("object" === typeof myblob) {
		if ((CSL.TERMINAL_PUNCTUATION.indexOf(chr) > -1 && 
			 CSL.TERMINAL_PUNCTUATION.slice(0, -1).indexOf(myblob.strings.prefix.slice(0, 1)) > -1)) {
			myblob.strings.prefix = myblob.strings.prefix.slice(1);
		} else if ("object" === typeof myblob.blobs) {
			CSL.Output.Queue.purgeNearsidePrefixChars(myblob.blobs[0], chr);
		}
	}
}

CSL.Output.Queue.purgeNearsidePrefixSpaces = function(myblob, chr) {
	//if (!chr) {
	//	return;
	//}
	if ("object" === typeof myblob) {
		if (" " === chr && " " === myblob.strings.prefix.slice(0, 1)) {
			myblob.strings.prefix = myblob.strings.prefix.slice(1);
		} else if ("object" === typeof myblob.blobs) {
			CSL.Output.Queue.purgeNearsidePrefixSpaces(myblob.blobs[0], chr);
		}
	}
}

CSL.Output.Queue.purgeNearsideSuffixSpaces = function(myblob, chr) {
	if ("object" === typeof myblob) {
		if (" " === chr && " " === myblob.strings.suffix.slice(-1)) {
			myblob.strings.suffix = myblob.strings.suffix.slice(0, -1);
		} else if ("object" === typeof myblob.blobs) {
			if (!chr) {
				chr = myblob.strings.suffix.slice(-1);
			}
			chr = CSL.Output.Queue.purgeNearsideSuffixSpaces(myblob.blobs[myblob.blobs.length - 1], chr);
		} else {
			chr = myblob.strings.suffix.slice(-1);
		}
	}
	return chr;
}

CSL.Output.Queue.adjustPunctuation = function (state, myblobs, stk, finish) {
	var chr, suffix, dpref, blob, delimiter, suffixX, dprefX, blobX, delimiterX, prefix, prefixX, dsuffX, dsuff, slast, dsufff, dsufffX, lastchr, firstchr, chr, exposed_suffixes, exposed;

	var TERMS = CSL.TERMINAL_PUNCTUATION.slice(0, -1);
	var TERM_OR_SPACE = CSL.TERMINAL_PUNCTUATION;
	var SWAPS = CSL.SWAPPING_PUNCTUATION;
	
	if (!stk) {
		stk = [{suffix: "", delimiter: ""}];
	}

	slast = stk.length - 1;

	delimiter = stk[slast].delimiter;
	dpref = stk[slast].dpref;
	dsuff = stk[slast].dsuff;
	dsufff = stk[slast].dsufff;
	prefix = stk[slast].prefix;
	suffix = stk[slast].suffix;
	blob = stk[slast].blob;

	if ("string" === typeof myblobs) {
		// Note that (1) the "suffix" variable is set 
		// non-nil only if it contains terminal punctuation;
		// (2) "myblobs" is a string in this case; (3) we
		// don't try to control duplicate spaces, because
		// if they're in the user-supplied string somehow, 
		// they've been put there by intention.
		if (suffix) {
			if (blob && 
				TERMS.indexOf(myblobs.slice(-1)) > -1 &&
				TERMS.indexOf(suffix) > -1) {
					blob.strings.suffix = blob.strings.suffix.slice(1);
			}
		}
		lastchr = myblobs.slice(-1);
		firstchr = myblobs.slice(0,1);
	} else {
		// Complete the move of a leading terminal punctuation 
		// from superior delimiter to suffix at this level,
		// to allow selective suppression.
		if (dpref) {
			for (var j = 0, jlen = myblobs.length - 1; j < jlen; j += 1) {
				var t = myblobs[j].strings.suffix.slice(-1);
				// print("hey: ["+j+"] ("+dpref+") ("+myblobs[0].blobs+")")

				if (TERMS.indexOf(t) === -1 ||
				    TERMS.indexOf(dpref) === -1) {
					// Drop duplicate space
					if (dpref !== " " || dpref !== myblobs[j].strings.suffix.slice(-1)) {
						myblobs[j].strings.suffix += dpref;						
					}
				}
			}
		}

		// For ParentalSuffixPrefixUphill
		if (suffix === " ") {
			CSL.Output.Queue.purgeNearsideSuffixSpaces(myblobs[myblobs.length - 1], " ");
		}
		var lst = [];
		for (var i = 0, ilen = myblobs.length - 1; i < ilen; i += 1) {
			var doblob = myblobs[i];
			var following_prefix = myblobs[i + 1].strings.prefix;
			var chr = false;
			// A record of the suffix leading character
			// nearest to each empty delimiter, for use
			// in comparisons in the next function.
			var ret = CSL.Output.Queue.purgeNearsideSuffixSpaces(doblob, chr);
			if (!dsuff) {
				lst.push(ret);
			} else {
				lst.push(false);
			}
		}

		// For ParentalSuffixPrefixDownhill
		chr = false;
		for (var i = 1, ilen = myblobs.length; i < ilen; i += 1) {
			var doblob = myblobs[i];
			var chr = "";
			var preceding_suffix = myblobs[i - 1].strings.suffix;
			if (dsuff === " ") {
				chr = dsuff;
			} else if (preceding_suffix) {
				chr = preceding_suffix.slice(-1);
			} else if (lst[i - 1]) {
				chr = lst[i - 1];
			}
			CSL.Output.Queue.purgeNearsidePrefixSpaces(doblob, chr);
		}
		if (dsufff) {
			CSL.Output.Queue.purgeNearsidePrefixSpaces(myblobs[0], " ");
		} else if (prefix === " ") {
			CSL.Output.Queue.purgeNearsidePrefixSpaces(myblobs[0], " ");
		}

		// Descend down the nearest blobs until we run into
		// a string blob or a prefix, and if we find a
		// prefix with an initial character that conflicts
		// with the lastchr found so far, quash the prefix char.
		for (var i = 0, ilen = myblobs.length; i < ilen; i += 1) {
			var doblob = myblobs[i];

			CSL.Output.Queue.purgeNearsidePrefixChars(doblob, lastchr);
			
			// Prefix and suffix
			if (i === 0) {
				if (prefix) {
					if (doblob.strings.prefix.slice(0, 1) === " ") {
						//doblob.strings.prefix = doblob.strings.prefix.slice(1);
					}
				}
			}
			
			if (dsufff) {
				if (doblob.strings.prefix) {
					if (i === 0) {
						if (doblob.strings.prefix.slice(0, 1) === " ") {
							//doblob.strings.prefix = doblob.strings.prefix.slice(1);
						}
					}
				}
			}
			if (dsuff) {
				if (i > 0) {
					if (doblob.strings.prefix.slice(0, 1) === " ") {
						//doblob.strings.prefix = doblob.strings.prefix.slice(1);
					}
				}
			}

			if (i < (myblobs.length - 1)) {
				// Migrate any leading terminal punctuation on a subsequent
				// prefix to the current suffix, iff the
				// (remainder of the) intervening delimiter is empty.
				// Needed for CSL of the Chicago styles.
				var nextprefix = myblobs[i + 1].strings.prefix;
				if (!delimiter) {
					if (nextprefix) {
						var nxtchr = nextprefix.slice(0, 1);
						if (SWAPS.indexOf(nxtchr) > -1) {
							myblobs[i + 1].strings.prefix = nextprefix.slice(1);
							if (TERMS.indexOf(nxtchr) === -1 ||
								(TERMS.indexOf(nxtchr) > -1 &&
								 TERMS.indexOf(doblob.strings.suffix.slice(-1)) === -1)) {
									 doblob.strings.suffix += nxtchr;
							}
						} else if (nxtchr === " " &&
									doblob.strings.suffix.slice(-1) === " ") {
							doblob.strings.suffix = doblob.strings.suffix.slice(0, -1);
						}
					}
				}
			}

			// If duplicate punctuation on superior suffix,
			// quash on superior object.
			if (i === (myblobs.length - 1)) {
				if (suffix) {
					if (doblob.strings.suffix && 
//						(suffix === doblob.strings.suffix.slice(-1) ||
						 (TERMS.indexOf(suffix) > -1 &&
						  TERMS.indexOf(doblob.strings.suffix.slice(-1)) > -1)) {
							blob.strings.suffix = blob.strings.suffix.slice(1);
					}
				}
			}

			// Run strip-periods.  This cleans affected field
			// content before is is processed by the first
			// "string" === typeof function above, at the next
			// iteration.
			if ("string" === typeof doblob.blobs && doblob.blobs) {
				for (var ppos = doblob.decorations.length - 1; ppos > -1; ppos += -1) {
					var params = doblob.decorations[ppos];
					if (params[0] === "@strip-periods" && params[1] === "true") {
						doblob.blobs = state.fun.decorate[params[0]][params[1]](state, doblob.blobs);
						doblob.decorations = doblob.decorations.slice(0, ppos).concat(doblob.decorations.slice(ppos + 1));
					}
				}
			}

			// Swap punctuation into quotation marks as required.
			//if (i === (myblobs.length - 1) && state.getOpt('punctuation-in-quote')) {
			if (state.getOpt('punctuation-in-quote')) {
				var decorations = doblob.decorations;
				for (var j = 0, jlen = decorations.length; j < jlen; j += 1) {
					if (decorations[j][0] === '@quotes' && decorations[j][1] === 'true') {
						var swapchar = doblob.strings.suffix.slice(0, 1);
						var swapblob = false;
						if (SWAPS.indexOf(swapchar) > -1) {
							swapblob = doblob;
						} else if (SWAPS.indexOf(suffix) > -1 && i === (myblobs.length - 1)) {
							swapchar = suffix;
							swapblob = blob;
						} else {
							swapchar = false;
						}
						// This reflects Chicago 16th.
						if (swapchar) {
							// For both possible case, if ending punctuation is 
							// not in SWAPS, add the swapchar.
							// Otherwise add the swapchar only if ending punctuation 
							// is in TERMS, and the swapchar is in SWAPS and not in TERMS.
							//
							// Code could do with some pruning, but that's the logic of it.
							if ("string" === typeof doblob.blobs) {
								if (SWAPS.indexOf(doblob.blobs.slice(-1)) === -1 ||
								   (TERMS.indexOf(doblob.blobs.slice(-1)) > -1 &&
									SWAPS.indexOf(swapchar) > -1 &&
									TERMS.indexOf(swapchar) === -1)) {
										doblob.blobs += swapchar;
								}
							} else {
								if (SWAPS.indexOf(doblob.blobs.slice(-1)[0].strings.suffix.slice(-1)) === -1 ||
									(TERMS.indexOf(doblob.blobs.slice(-1)[0].strings.suffix.slice(-1)) > -1 &&
									 SWAPS.indexOf(swapchar) > -1 &&
									 TERMS.indexOf(swapchar) === -1)) {
										 doblob.blobs.slice(-1)[0].strings.suffix += swapchar;
								}
							}
							swapblob.strings.suffix = swapblob.strings.suffix.slice(1);
						}
					}
				}
			}

			// Prepare variables for the sniffing stack, for use
			// in the next recursion.

			if (i === (myblobs.length - 1)) {
				// If last blob in series, use superior suffix if current
				// level has none.
				if (doblob.strings.suffix) {
					suffixX = doblob.strings.suffix.slice(0, 1);
					blobX = doblob;
				} else {
					suffixX = stk[stk.length - 1].suffix;
					blobX = stk[stk.length - 1].blob;
				}
			} else {
				// If NOT last blob in series, use only the current
				// level suffix for sniffing.
				if (doblob.strings.suffix) {
					suffixX = doblob.strings.suffix.slice(0, 1);
					blobX = doblob;
  				} else {
					suffixX = "";
					blobX = false;
				}
				
			}

			// Use leading suffix char for sniffing only if it
			// is a terminal punctuation character.
			if (SWAPS.concat([" "]).indexOf(suffixX) === -1) {
			//if (SWAPS.indexOf(suffixX) === -1) {
				suffixX = "";
				blobX = false;
			}

			// Use leading delimiter char for sniffing only if it
			// is a terminal punctuation character.
			if (doblob.strings.delimiter && 
				doblob.blobs.length > 1) {
				dprefX = doblob.strings.delimiter.slice(0, 1);
				if (SWAPS.concat([" "]).indexOf(dprefX) > -1) {
				//if (SWAPS.indexOf(dprefX) > -1) {
					doblob.strings.delimiter = doblob.strings.delimiter.slice(1);
				} else {
					dprefX = "";
				}
			} else {
				dprefX = "";
			}
			
			if (doblob.strings.prefix) {
				if (doblob.strings.prefix.slice(-1) === " ") {
					// Marker copy only, no slice at this level before descending.
					prefixX = " ";
				} else {
					prefixX = "";
				}
			} else {
				if (i === 0) {
					prefixX = prefix;					
				} else {
					prefixX = "";
				}
			}

			if (dsuff) {
				dsufffX = dsuff;
			} else {
				if (i === 0) {
					dsufffX = dsufff;					
				} else {
					dsufffX = "";
				}
			}
			if (doblob.strings.delimiter) {
				if (doblob.strings.delimiter.slice(-1) === " " &&
					"object" === typeof doblob.blobs && doblob.blobs.length > 1) {
					   dsuffX = doblob.strings.delimiter.slice(-1);
				} else {
					dsuffX = "";						
				}
			} else {
				dsuffX = "";					
			}
			
			delimiterX = doblob.strings.delimiter;

			// Push variables to stack and recurse.
			stk.push({suffix: suffixX, dsuff:dsuffX, blob:blobX, delimiter:delimiterX, prefix:prefixX, dpref: dprefX, dsufff: dsufffX});
			lastchr = CSL.Output.Queue.adjustPunctuation(state, doblob.blobs, stk);
		}
		
		// cmd_cite.js needs a report of the last character to be
		// rendered, to suppress extraneous trailing periods in
		// rare cases.
		if (myblobs && myblobs.length) {
			var last_suffix = myblobs[myblobs.length - 1].strings.suffix;
			if (last_suffix) {
				lastchr = last_suffix.slice(-1);
			}
		}
	}
	// Always pop the stk when returning, unless it's the end of the line
	// (return value is needed in cmd_cite.js, so that the adjusted
	// suffix can be extracted from the fake blob used at top level).
	if (stk.length > 1) {
		stk.pop();
	}
	// Are these needed?
	state.tmp.last_chr = lastchr;
	return lastchr;
};

