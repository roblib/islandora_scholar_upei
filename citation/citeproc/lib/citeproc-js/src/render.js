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

// The new-look output queue.

// This should be designed so that a method .string() is
// available on all blobs.  Then for selective rendering, we
// can just run the method on the blob, and it's done.

CSL.Render = function (state) {
	//
	// Make state object available.
	this.state = state;
	//
	// Tokens are named bundles of decorations, affixes and delimiters,
	// with a structure similar to that of the main style tokens.
	// We avoid CSL.Stack() here to save overhead.
	this.formats = [
		{empty: new CSL.Token("empty")}
	];
	this.formats_lastpos = 0;
	//
	// The nested output queue.
	this.top = new CSL.Blob(false, false, "top");
	//
	// Current always returns the current blob with dependent children.
	this.current = [this.top];
	this.current_lastpos = 0;
	// Flag to control whether offset characters are calculated
	this.calculate_offset = false;
	this.offset = 0;
};

CSL.Render.overlayStrings = function (target, stringtok) {
	var attr;
	for (attr in stringtok.strings) {
		if (stringtok.strings.hasOwnProperty(attr)) {
			target.strings[attr] = stringtok.strings[attr];
		}
	}
};

CSL.Render.prototype.getToken = function (name) {
	return this.formats[this.formats_lastpos][name];
};

CSL.Render.prototype.getStrings = function (name) {
	return this.formats[this.formats_lastpos][name].strings;
};

CSL.Render.prototype.getDecorations = function (name) {
	return this.formats[this.formats_lastpos][name].decorations;
};

//
// If name only, create an empty token of "name" in formats.
// If name + addtok, create a new tok formatted as addtok and add to formats as name.
CSL.Render.prototype.addToken = function (name, tok) {
	var newtok, attr;
	//
	//if (this.formats[this.formats_lastpos][name]) {
	//	throw "CSL error: token " + name + " exists at level: " + this.formats.length;
	//}
	newtok = new CSL.Token(name);
	CSL.Render.overlayStrings(newtok, tok);
	newtok.decorations = tok.decorations;
	this.formats[this.formats_lastpos][name] = newtok;
};

//
// name is a string, which may be the same as the name of base, decorations,
//   or strings.
// base is a string, the name of a token to use as the base
// decorations is a the name of a token containing decorations
// strings is the name of a token (possibly same as above) containing strings.
//
// The decorations and strings are overlaid onto base, and the resulting
// token is added to formats as name.  The base, decorations and strings
// tokens may be overwritten by name, but are not otherwise affected.
//
CSL.Render.prototype.mergeToken = function (name, base, decorations, strings) {
	var newtok, stringtok, decortok;
	newtok = CSL.Token(name);
	base = this.formats[this.formats_lastpos][base];
	CSL.Render.overlayStrings(newtok, base);
	newtok.decorations = base.decorations;
	if (decorations) {
		decortok = this.formats[this.formats_lastpos][decorations];
		newtok.decorations = decortok.decorations;
	}
	if (strings) {
		stringtok = this.formats[this.formats_lastpos][strings];
		CSL.Render.overlayStrings(newtok, stringtok);
	}
};


CSL.Render.prototype.newFormats = function (name, token) {
	var tokenstore = {};
	tokenstore[name] = token;
	this.formats.push(tokenstore);
	this.openLevel(name);
};


CSL.Render.prototype.oldFormats = function (name) {
	this.closeLevel(name);
	this.formats.pop();
};

CSL.Render.prototype.openLevel = function (name, ephemeral) {
	var blob, curr, x, has_ephemeral;

	//if (!this.formats[this.formats_lastpos][name]) {
	//	throw "CSL processor error: call to nonexistent format token \"" + name + "\"";
	//}

	// delimiter, prefix, suffix, decorations from token
	blob = new CSL.Blob(this.formats[this.formats_lastpos][name], false, name);
	if (this.calculate_offset && blob.strings.prefix.length) {
		this.offset += blob.strings.prefix.length;
	}
	if (this.calculate_offset && blob.strings.suffix.length) {
		this.offset += blob.strings.suffix.length;
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
CSL.Render.prototype.closeLevel = function (name) {
	if (name && name !== this.current.value().levelname) {
		CSL.error("Level mismatch error:  wanted " + name + " but found " + this.current.value().blobs[this.current.value().blobs.length - 1].levelname);
	}
	this.current.pop();
};



CSL.render = CSL.Render();
