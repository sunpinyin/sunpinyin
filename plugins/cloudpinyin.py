#!/usr/bin/env python
# -*- coding: utf-8 -*-

import re
from ast import literal_eval
from threading import Thread, Lock
from datetime import datetime, timedelta
from urllib2 import urlopen, unquote

__NAME__ = 'Cloud Pinyin Plugin'
__AUTHOR__ = 'Etrnls'
__EMAIL__ = 'Etrnls@gmail.com'
__DESCRIPTION__ = 'Pinyin input method based on cloud pinyin services'

_waittime = 1

class _Cache(object):
	def __init__(self):
		self._data = dict()
		self._lock = Lock()

	def _clean(self):
		now = datetime.now()
		expires = []
		for preedit, data in self._data.iteritems():
			for rank, (timestamp, candidate) in data.iteritems():
				if now - timestamp > timedelta(minutes = 10):
					expires.append((preedit, rank))
		for preedit, rank in expires:
			del self._data[preedit][rank]

	def lookup(self, preedit):
		self._lock.acquire()
		if preedit in self._data:
			result = self._data[preedit]
			self._lock.release()
			return result
		self._lock.release()

	def insert(self, preedit, rank, candidate):
		self._lock.acquire()
		self._clean()
		if preedit not in self._data:
			self._data[preedit] = {rank : (datetime.now(), candidate)}
		else:
			self._data[preedit][rank] = (datetime.now(), candidate)
		self._lock.release()

_cache = _Cache()

class _Fetcher(object):
	_MAX_RETRY = 5

	def __init__(self, rank):
		self._rank = rank
		self._key = None
		Thread(target = self._fetch_key).start()
		self._fetching = set()
		self._fetching_lock = Lock()

	def _fetch_key(self):
		for retry in range(0, self._MAX_RETRY):
			try:
				self._key = self._do_fetch_key()
				if self._key:
					print 'Key %s fetched for %s' % (self._key, self)
					return
			except:
				pass

	def fetch(self, preedit):
		self._fetching_lock.acquire()
		if preedit in self._fetching:
			self._fetching_lock.release()
			return
		self._fetching.add(preedit)
		self._fetching_lock.release()

		for retry in range(0, self._MAX_RETRY):
			try:
				candidate = self._do_fetch(preedit)
				if candidate:
					print 'Candidate %s fetched for %s' % (candidate, self)
					_cache.insert(preedit, self._rank, candidate)
					break
			except:
				pass

		self._fetching_lock.acquire()
		self._fetching.remove(preedit)
		self._fetching_lock.release()

class _SogouFetcher(_Fetcher):
	_URL = 'http://web.pinyin.sogou.com/api/py?key=%s&query=%s'
	_KEY_URL = 'http://web.pinyin.sogou.com/web_ime/patch.php'

	def _do_fetch_key(self):
		data = urlopen(self._KEY_URL).read()
		return re.search('"(.+)"', data).group(1)

	def _do_fetch(self, preedit):
		if not self._key:
			return
		result = urlopen(self._URL % (self._key, preedit)).read()
		return re.search('"(.+?)\xef\xbc\x9a', unquote(result)).group(1)

class _QQFetcher(_Fetcher):
	_URL = 'http://ime.qq.com/fcgi-bin/getword?key=%s&q=%s'
	_KEY_URL = 'http://ime.qq.com/fcgi-bin/getkey'

	def _do_fetch_key(self):
		data = urlopen(self._KEY_URL).read()
		result = literal_eval(data)
		if result['ret'] != 'suc':
			return
		return result['key']

	def _do_fetch(self, preedit):
		data = urlopen(self._URL % (self._key, preedit)).read()
		result = literal_eval(data)
		if result['ret'] != 'suc':
			return
		return result['rs'][0]

class _GoogleFetcher(_Fetcher):
	_URL = 'http://www.google.com/inputtools/request?ime=pinyin&text=%s'

	def _do_fetch_key(self):
		pass

	def _do_fetch(self, preedit):
		result = literal_eval(urlopen(self._URL % preedit).read())
		if result[0] != 'SUCCESS':
			return
		return result[1][0][1][0]

_fetchers = [_SogouFetcher(0), _QQFetcher(1), _GoogleFetcher(1)]

def provide_candidates(preedit):
	preedit = preedit.replace(' ', '')
	data = _cache.lookup(preedit)
	if data:
		candidates = []
		data = sorted(data.iteritems(), key = lambda x:x[0])
		rank = 0
		for _, (timestamp, candidate) in data:
			candidate = candidate.decode('utf-8')
			duplicate = False
			for r, c in candidates:
				if c == candidate:
					duplicate = True
					break
			if not duplicate:
				candidates.append((rank, candidate))
				rank += 1
		return candidates
	for fetcher in _fetchers:
		Thread(target = fetcher.fetch, args = (preedit, )).start()
	return _waittime

