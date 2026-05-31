import { useEffect, useState } from "react";
import ReactMarkdown from "react-markdown";
import { Link, NavLink, Route, Routes, useParams } from "react-router-dom";

type ReleaseManifest = {
  version: string;
  summary: string;
  notes_url: string;
  firmware_url: string;
  published_at: string;
  sha256: string;
  size: number;
};

type ReleaseEntry = {
  version: string;
  title: string;
  published_at: string;
  summary: string;
  notes_path: string;
  notes_url: string;
  firmware_url: string;
};

type ReleaseIndex = {
  releases: ReleaseEntry[];
};

const releaseDownloadBase =
  "https://github.com/ThingEngineer/SnowLeopard/releases/download";

function getReleaseDataBase() {
  const fallbackBase = import.meta.env.BASE_URL || "/";
  const runtimePath =
    typeof window === "undefined"
      ? fallbackBase
      : window.location.pathname || fallbackBase;
  const baseWithSlash = runtimePath.endsWith("/")
    ? runtimePath
    : `${runtimePath}/`;
  return `${baseWithSlash.replace(/\/+$/, "/")}release-data`;
}

const releaseDataBase = getReleaseDataBase();

function compareReleaseVersions(a: string, b: string) {
  return b.localeCompare(a, undefined, {
    numeric: true,
    sensitivity: "base",
  });
}

function normalizeGitTag(version: string) {
  return version.startsWith("v") ? version : `v${version}`;
}

function getFallbackFirmwareUrl(version: string) {
  const tag = normalizeGitTag(version);
  return `${releaseDownloadBase}/${tag}/snowleopard-${tag}.bin`;
}

function resolveFirmwareUrl(url: string | undefined, version: string) {
  if (url) {
    try {
      const parsed = new URL(url);
      if (parsed.protocol === "https:" || parsed.protocol === "http:") {
        return parsed.toString();
      }
    } catch {
      // Fall through to the canonical GitHub release URL.
    }
  }

  return getFallbackFirmwareUrl(version);
}

async function fetchJson<T>(path: string): Promise<T> {
  const response = await fetch(path, { cache: "no-store" });
  if (!response.ok) {
    throw new Error(`Request failed for ${path}`);
  }
  return response.json() as Promise<T>;
}

function formatDate(value: string) {
  if (!value) {
    return "Pending date";
  }
  const date = new Date(value);
  if (Number.isNaN(date.getTime())) {
    return value;
  }
  return date.toLocaleDateString(undefined, {
    year: "numeric",
    month: "long",
    day: "numeric",
  });
}

function useReleaseData() {
  const [currentRelease, setCurrentRelease] = useState<ReleaseManifest | null>(
    null,
  );
  const [releaseIndex, setReleaseIndex] = useState<ReleaseEntry[]>([]);
  const [loading, setLoading] = useState(true);
  const [error, setError] = useState("");

  useEffect(() => {
    let cancelled = false;

    async function load() {
      try {
        setLoading(true);
        const [current, index] = await Promise.all([
          fetchJson<ReleaseManifest>(`${releaseDataBase}/current.json`),
          fetchJson<ReleaseIndex>(`${releaseDataBase}/releases/index.json`),
        ]);
        if (!cancelled) {
          const sortedReleases = [...(index.releases ?? [])].sort((a, b) => {
            const dateDiff =
              new Date(b.published_at).getTime() -
              new Date(a.published_at).getTime();
            if (!Number.isNaN(dateDiff) && dateDiff !== 0) {
              return dateDiff;
            }
            return compareReleaseVersions(a.version, b.version);
          });

          setCurrentRelease(current);
          setReleaseIndex(sortedReleases);
          setError("");
        }
      } catch (loadError) {
        if (!cancelled) {
          setError("We could not load release information right now.");
        }
      } finally {
        if (!cancelled) {
          setLoading(false);
        }
      }
    }

    load();
    return () => {
      cancelled = true;
    };
  }, []);

  return { currentRelease, releaseIndex, loading, error };
}

function App() {
  const releaseData = useReleaseData();

  return (
    <div className="shell">
      <header className="masthead">
        <div className="brand-block">
          <div className="eyebrow">SnowLeopard firmware releases</div>
          <h1>Stay up to date with SnowLeopard firmware.</h1>
          <p className="lede">
            See the latest version, what changed, and where to download each
            release. This page is focused on helping users understand and apply
            updates quickly.
          </p>
        </div>
        <nav className="topnav" aria-label="Primary">
          <NavLink to="/">Overview</NavLink>
          <NavLink to="/releases">Release notes</NavLink>
          <a
            href="https://github.com/ThingEngineer/SnowLeopard"
            target="_blank"
            rel="noopener noreferrer"
          >
            Repository
          </a>
        </nav>
      </header>

      <main className="content">
        <Routes>
          <Route path="/" element={<OverviewPage {...releaseData} />} />
          <Route path="/releases" element={<ReleasesPage {...releaseData} />} />
          <Route
            path="/releases/:version"
            element={<ReleaseNotesPage {...releaseData} />}
          />
        </Routes>
      </main>
    </div>
  );
}

function OverviewPage({
  currentRelease,
  releaseIndex,
  loading,
  error,
}: {
  currentRelease: ReleaseManifest | null;
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  const newest = releaseIndex[0];
  const currentVersion =
    currentRelease?.version ?? newest?.version ?? "loading";
  const currentSummary =
    currentRelease?.summary ??
    newest?.summary ??
    "Release details will appear here soon.";
  const publishedAt = currentRelease?.published_at || newest?.published_at;

  return (
    <>
      <section className="hero-panel">
        <div className="metric-card accent">
          <span className="metric-label">Latest firmware version</span>
          <strong>{currentVersion}</strong>
          <span>{currentSummary}</span>
          <span className="metric-subtle">
            {publishedAt
              ? `Published ${formatDate(publishedAt)}`
              : "Publish date pending"}
          </span>
        </div>
        <div className="metric-card">
          <span className="metric-label">What is new</span>
          <strong>{newest?.title ?? "Checking latest release"}</strong>
          <span>
            {newest?.summary ?? "Loading the latest release summary..."}
          </span>
          <div className="metric-actions">
            {newest ? (
              <Link to={`/releases/${newest.version}`}>Read release notes</Link>
            ) : null}
            <Link to="/releases">Browse all releases</Link>
          </div>
        </div>
      </section>

      <section className="section-grid">
        <article className="panel">
          <h2>Quick update tips</h2>
          <ul className="quick-list">
            <li>
              Use the release notes to see whether a version is relevant to your
              setup.
            </li>
            <li>
              Update from the device Settings page when a newer version appears.
            </li>
            <li>
              Keep power and Wi-Fi stable while firmware is downloading and
              installing.
            </li>
          </ul>
        </article>

        <article className="panel">
          <h2>Need help?</h2>
          <p>
            New to SnowLeopard or troubleshooting an update? Start with the user
            guide, then check release notes for version-specific details.
          </p>
          <div className="help-links">
            <a
              href="https://github.com/ThingEngineer/SnowLeopard/blob/main/docs/USER-GUIDE.md"
              target="_blank"
              rel="noopener noreferrer"
            >
              Open user guide
            </a>
            <a
              href="https://github.com/ThingEngineer/SnowLeopard/blob/main/docs/QUICKSTART.md"
              target="_blank"
              rel="noopener noreferrer"
            >
              Open quickstart
            </a>
          </div>
        </article>
      </section>

      {loading ? <p>Loading release data...</p> : null}
      {error ? <p className="error-text">{error}</p> : null}
    </>
  );
}

function ReleasesPage({
  releaseIndex,
  loading,
  error,
}: {
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  const releaseCountLabel =
    releaseIndex.length === 1
      ? "1 version available"
      : `${releaseIndex.length} versions available`;

  return (
    <section className="panel">
      <div className="panel-head">
        <h2>All releases</h2>
        <span>{releaseCountLabel}</span>
      </div>
      <p className="section-intro">
        Browse every published SnowLeopard version, read what changed, and
        download update files if needed.
      </p>
      {loading ? <p>Loading releases...</p> : null}
      {error ? <p className="error-text">{error}</p> : null}
      {!loading && !error ? (
        <div className="release-list">
          {releaseIndex.map((release) => {
            const firmwareUrl = resolveFirmwareUrl(
              release.firmware_url,
              release.version,
            );

            return (
              <article className="release-row" key={release.version}>
                <div>
                  <div className="release-meta">
                    <span>{release.version}</span>
                    <span>{formatDate(release.published_at)}</span>
                  </div>
                  <h3>{release.title}</h3>
                  <p>{release.summary}</p>
                </div>
                <div className="release-actions">
                  <Link to={`/releases/${release.version}`}>
                    Read what changed
                  </Link>
                  <a
                    href={firmwareUrl}
                    target="_blank"
                    rel="noopener noreferrer"
                  >
                    Download update file
                  </a>
                </div>
              </article>
            );
          })}
        </div>
      ) : null}
    </section>
  );
}

function ReleaseNotesPage({
  releaseIndex,
  loading,
  error,
}: {
  releaseIndex: ReleaseEntry[];
  loading: boolean;
  error: string;
}) {
  const { version } = useParams();
  const release = releaseIndex.find((entry) => entry.version === version);
  const [markdown, setMarkdown] = useState("");
  const [notesError, setNotesError] = useState("");

  useEffect(() => {
    let cancelled = false;

    async function loadMarkdown() {
      if (!release?.notes_path) {
        setMarkdown("");
        return;
      }
      try {
        const response = await fetch(
          `${releaseDataBase}/${release.notes_path}`,
          { cache: "no-store" },
        );
        if (!response.ok) {
          throw new Error("We could not load these release notes right now.");
        }
        const content = await response.text();
        if (!cancelled) {
          setMarkdown(content);
          setNotesError("");
        }
      } catch (loadError) {
        if (!cancelled) {
          setNotesError(
            loadError instanceof Error
              ? loadError.message
              : "We could not load these release notes right now.",
          );
        }
      }
    }

    loadMarkdown();
    return () => {
      cancelled = true;
    };
  }, [release]);

  return (
    <section className="panel notes-panel">
      <div className="panel-head">
        <div>
          <h2>{release?.title ?? version ?? "Release details"}</h2>
          {release ? (
            <div className="release-meta">
              <span>{release.version}</span>
              <span>{formatDate(release.published_at)}</span>
            </div>
          ) : null}
        </div>
        <Link to="/releases">Back to all releases</Link>
      </div>
      {loading ? <p>Loading release details...</p> : null}
      {error ? <p className="error-text">{error}</p> : null}
      {!loading && !error && !release ? (
        <p className="error-text">
          We could not find a release for version {version}.
        </p>
      ) : null}
      {notesError ? <p className="error-text">{notesError}</p> : null}
      {markdown ? (
        <div className="markdown-body">
          <ReactMarkdown>{markdown}</ReactMarkdown>
        </div>
      ) : null}
      {!loading && !error && !notesError && release && !markdown ? (
        <p>Detailed notes for this release will be posted soon.</p>
      ) : null}
    </section>
  );
}

export default App;
